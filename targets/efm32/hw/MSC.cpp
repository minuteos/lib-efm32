/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/MSC.cpp
 */

#include "MSC.h"

#include <ld_symbols.h>

#define MYDBG(...)  DBGCL("FLASH", __VA_ARGS__)

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

    MYDBG("Writing %u bytes at %08X", data.Length(), addr);

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
            MYDBG("Unaligned initial %d bytes, writing %08X at %08X", 4 - off, wr | mask, addr);
        }

        if (length < 4)
        {
            // final unaligned bytes - compensate by replacing top bits with ones, set length to 4
            // length == 1 => wr = FFFFFF00 ... 3 => wr = FF221100
            mask = ~0u << length * 8;
            MYDBG("Unaligned final %d bytes, writing %08X at %08X", length, wr | mask, addr);
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
    constexpr uint32_t PageMask = ~(PageSize - 1);
    constexpr uint32_t PageWords = PageSize / sizeof(uint32_t);
    uint32_t* p = (uint32_t*)((uint32_t)address & PageMask);
    uint32_t* end = (uint32_t*)(((uint32_t)address + length - 1) & PageMask);

    if (p < &__boot_end)
        p = &__boot_end;

    uint32_t retry = 3;

    while (p < end)
    {
        uint32_t* page = p;
        uint32_t* pageEnd = p + PageWords;
        do
        {
            if (*p++ != ~0u)
                break;
        } while (p < pageEnd);

        if (p[-1] == ~0u)
        {
            MYDBG("Page at %08X-%08X is empty", page, p);
            ASSERT(((uint)p & ~PageMask) == 0);
            retry = 3;
            continue;
        }

        if (!--retry)
        {
            MYDBG("FAILED TO ERASE PAGE AT %08X!!!", page);
            return false;
        }

        MYDBG("Erasing page at %08X-%08X", page, pageEnd);

        UnlockFlash();

        ADDRB = (uint32_t)p;
        WRITECMD = MSC_WRITECMD_LADDRIM | MSC_WRITECMD_ERASEPAGE;

        LockFlash();

        p = page;
    }

    return true;
}
