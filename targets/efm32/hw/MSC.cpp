/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/MSC.cpp
 */

#include "MSC.h"

#include <hw/SCB.h>

#include <ld_symbols.h>

//#define MSC_TRACE 1

#define MYDBG(...)  DBGCL("FLASH", __VA_ARGS__)

#if MSC_TRACE
#define MYTRACE MYDBG
#else
#define MYTRACE(...)
#endif

static constexpr uint32_t PageSize = FLASH_PAGE_SIZE;
static constexpr uint32_t PageMask = ~(PageSize - 1);
static constexpr uint32_t PageWords = PageSize / sizeof(uint32_t);

bool _MSC::WriteWord(const volatile void* ptr, uint32_t value)
{
    if (ptr < &__boot_end)
        return false;

    UnlockFlash();

    ADDRB = (uint32_t)ptr;
    WDATA = value;
    WRITECMD = MSC_WRITECMD_LADDRIM | MSC_WRITECMD_WRITEONCE;
    Sync();

    if (*(const uint*)ptr != value)
    {
        // errata FLASH_E201 - first write may not succeed
        ADDRB = (uint32_t)ptr;
        WDATA = value;
        WRITECMD = MSC_WRITECMD_LADDRIM | MSC_WRITECMD_WRITEONCE;
        Sync();
    }

    LockFlash();

    return *(const uint32_t*)ptr == value;
}

bool _MSC::Write(const volatile void* address, Span data)
{
    if (address < &__boot_end)
        return false;

    uint32_t addr = (uint32_t)address;
    uint32_t wdata = (uint32_t)data.begin();
    uint32_t length = (uint32_t)data.Length();
    bool res = true;

    MYTRACE("Writing %u bytes at %08X", data.Length(), addr);

    UnlockFlash();

    while (length)
    {
        uint32_t wr = *(const uint32_t*)wdata;	// wr = 33221100
        uint32_t mask = 0;					// fill mask
        if (uint32_t off = addr & 3)
        {
            // first unaligned bytes - compensate by prepending ones and offsetting length
            // off == 1 => wr = 221100FF ... 3 => wr = 00FFFFFF
            wr <<= off * 8;
            mask = MASK(off * 8);
            wdata -= off;
            length += off;
            addr -= off;
            MYTRACE("Unaligned initial %d bytes, writing %08X at %08X", 4 - off, wr | mask, addr);
        }

        if (length < 4)
        {
            // final unaligned bytes - compensate by replacing top bits with ones, set length to 4
            // length == 1 => wr = FFFFFF00 ... 3 => wr = FF221100
            mask = ~0u << length * 8;
            MYTRACE("Unaligned final %d bytes, writing %08X at %08X", length, wr | mask, addr);
            length = 4;
        }

        wr |= mask;
        ADDRB = addr;
        WDATA = wr;
        WRITECMD = MSC_WRITECMD_LADDRIM | MSC_WRITECMD_WRITEONCE;
        Sync();
        if ((*(uint32_t*)addr | mask) != wr)
        {
            // errata FLASH_E201 - first write after reboot may fail, retry once
            ADDRB = addr;
            WDATA = wr;
            WRITECMD = MSC_WRITECMD_LADDRIM | MSC_WRITECMD_WRITEONCE;
            Sync();

            if ((*(uint32_t*)addr | mask) != wr)
            {
                res = false;
                break;
            }
        }
        addr += 4;
        wdata += 4;
        length -= 4;
    }

    LockFlash();

    return res;
}

bool _MSC::Erase(const volatile void* address, uint32_t length)
{
    uint32_t* p = (uint32_t*)((uint32_t)address & PageMask);
    uint32_t* end = (uint32_t*)(((uint32_t)address + length - 1) & PageMask);

    if (p < &__boot_end)
        p = &__boot_end;

    uint32_t retry = 3;

    while (p < end)
    {
        if (IsErased(p))
        {
            MYDBG("Page at %08X-%08X is empty", p, p + PageWords);
            p += PageWords;
            retry = 3;
            continue;
        }

        if (!--retry)
        {
            MYDBG("FAILED TO ERASE PAGE AT %08X!!!", p);
            return false;
        }

        MYDBG("Erasing page at %08X-%08X", p, p + PageWords);

        UnlockFlash();

        ADDRB = (uint32_t)p;
        WRITECMD = MSC_WRITECMD_LADDRIM | MSC_WRITECMD_ERASEPAGE;

        LockFlash();
    }

    return true;
}

bool _MSC::IsErased(const volatile void* page)
{
    const uint32_t* p = (const uint32_t*)page;
    const uint32_t* e = p + PageWords;

    do
    {
        if (*p++ != ~0u)
        {
            return false;
        }
    } while (p < e);

    return true;
}

__attribute__((section(".data#")))
void _MSC::TryErasePageHelper()
{
    WRITECMD = MSC_WRITECMD_LADDRIM | MSC_WRITECMD_ERASEPAGE;
    SCB->WaitForInterrupt();
    if (Busy())
    {
        // the interrupt was not an erase complete interrupt
        WRITECMD = MSC_WRITECMD_ERASEABORT;
        while (Busy());
    }
}

bool _MSC::TryErasePage(const volatile void* page)
{
    MYTRACE("Trying to erase page at %08X-%08X", page, (const uint32_t*)page + PageWords);

    UnlockFlash();

    IEN = MSC_IEN_ERASE;
    IFC = MSC_IFC_ERASE;
    SCB->DisableDeepSleep();
    SCB->EnableWake(MSC_IRQn);

    ADDRB = (uint32_t)page;
    // the erase command itself must be executed from RAM, otherwise the core halts on an interrupt
    TryErasePageHelper();

    SCB->DisableWake(MSC_IRQn);
    IEN = 0;

    LockFlash();

    if (!IsErased(page))
    {
        MYTRACE(STATUS & MSC_STATUS_ERASEABORTED ? "ERASE ABORTED" : "PAGE NOT ERASED");
        return false;
    }

    return true;
}

async(_MSC::ErasePage, const volatile void* address)
async_def()
{
    static struct EraseOperation
    {
        const volatile void* address;
        bool active, done;

        bool PreSleep(mono_t t, mono_t sleepTicks)
        {
            if (sleepTicks < MonoFromMilliseconds(1))
            {
                return false;
            }

            MYTRACE("TryErase: can sleep until tick %d, now %d", t + sleepTicks, MONO_CLOCKS);

            // try an erase operation, scheduling wakeup timer as with regular sleep
            CORTEX_SCHEDULE_WAKEUP(t + sleepTicks);

            done = MSC->TryErasePage(address);

            CORTEX_CLEAN_WAKEUP();

            if (done)
            {
                MYDBG("Successfully erase page @ %08X in %d ticks", address, MONO_CLOCKS - t);
            }
            return done;
        }
    } op = {};

    uint32_t* p = (uint32_t*)((uint32_t)address & ~(PageSize - 1));

    if (p < &__boot_end)
    {
        async_return(false);
    }

    if (IsErased(p))
    {
        async_return(true);
    }

    if (!await_acquire_sec(op.active, 1, 1))
    {
        async_return(false);
    }

    op.done = false;
    op.address = address;
    kernel::Scheduler::Current().AddPreSleepCallback(op, &EraseOperation::PreSleep);

    if (!await_signal_sec(op.done, 1))
    {
        kernel::Scheduler::Current().RemovePreSleepCallback(op, &EraseOperation::PreSleep);
    }

    op.active = false;
    async_return(op.done);
}
async_end
