/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/MSC.h
 */

#pragma once

#include <base/base.h>
#include <base/Span.h>

#include <kernel/kernel.h>

#undef MSC
#define MSC    CM_PERIPHERAL(_MSC, MSC_BASE)

class _MSC : public MSC_TypeDef
{
public:
    bool WriteWord(const volatile void* ptr, uint32_t value);
    bool Write(const volatile void* ptr, Span data);
    bool Erase(const volatile void* ptr, uint32_t length);
    bool TryErasePage(const volatile void* ptr);
    async(ErasePage, const volatile void* ptr);

private:
    static constexpr uint32_t PageSize = FLASH_PAGE_SIZE;

    bool Busy() { return STATUS & MSC_STATUS_BUSY; }
    void Sync() { while (Busy()); }

    void Unlock() { LOCK = MSC_LOCK_LOCKKEY_UNLOCK; }
    void UnlockFlash() { Unlock(); WRITECTRL = MSC_WRITECTRL_WREN; }
    void Lock() { LOCK = MSC_LOCK_LOCKKEY_LOCK; }
    void LockFlash() { WRITECTRL = _MSC_WRITECTRL_RESETVALUE; Lock(); }

    bool IsErased(const volatile void* page);

    void TryErasePageHelper();

    ALWAYS_INLINE void Configure()
    {
#ifdef _SILICON_LABS_32B_SERIES_1
        // use the "clear interrupts on read" semantics (reading IFC is the same as IFC = IF)
        // also enable bus faults when accessing disabled peripherals
        // silently ignoring these errors sometimes causes very hard to find bugs
        MSC->LOCK = MSC_LOCK_LOCKKEY_UNLOCK;
        MSC->CTRL |= MSC_CTRL_IFCREADCLEAR | MSC_CTRL_CLKDISFAULTEN;
        MSC->LOCK = MSC_LOCK_LOCKKEY_LOCK;
#endif
    }

    friend void _efm32_c_startup();
};
