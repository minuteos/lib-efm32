/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/nvram/Flash.h
 *
 * Flash interface for lib-nvram on EFM32
 */

#include <base/base.h>
#include <base/Span.h>

#include <kernel/kernel.h>

#include <hw/DEVINFO.h>
#include <hw/MSC.h>

namespace nvram
{

class Flash
{
public:
    static constexpr size_t PageSize = FLASH_PAGE_SIZE;

    ALWAYS_INLINE static Span GetRange() { return Span((const void*)FLASH_BASE, DEVINFO->FlashSize()); }

    static bool Write(const void* ptr, Span data) { return MSC->Write(ptr, data); }
    static bool WriteWord(const void* ptr, uint32_t word) { return MSC->WriteWord(ptr, word); }
    static void ShredWord(const void* ptr) { while (!MSC->WriteWord(ptr, 0)); }
    static bool Erase(Span range) { return MSC->Erase(range.Pointer(), range.Length()); }
    static async(ErasePageAsync, const void* ptr) { return async_forward(MSC->ErasePage, ptr); }
};

}
