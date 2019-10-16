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

#undef MSC
#define MSC    CM_PERIPHERAL(_MSC, MSC_BASE)

class _MSC : public MSC_TypeDef
{
public:
    bool WriteWord(const volatile void* ptr, uint32_t value);
    bool Write(const volatile void* ptr, Span data);
    bool Erase(const volatile void* ptr, uint32_t length);

private:
    static constexpr uint32_t PageSize = FLASH_PAGE_SIZE;

    bool Busy() { return STATUS & MSC_STATUS_BUSY; }
    void Sync() { while (Busy()); }

    void Unlock() { LOCK = MSC_UNLOCK_CODE; }
    void UnlockFlash() { Unlock(); WRITECTRL = MSC_WRITECTRL_WREN; }
    void Lock() { LOCK = 0; }
    void LockFlash() { WRITECTRL = 0; Lock(); }
};
