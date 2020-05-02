/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/DEVINFO.h
 */

#pragma once

#include <base/base.h>

#include <base/Span.h>

#undef DEVINFO
#define DEVINFO    CM_PERIPHERAL(_DEVINFO, DEVINFO_BASE)

class _DEVINFO : public DEVINFO_TypeDef
{
public:
    //! Gets the 48-bit Extended Unique Identifier of the device
    Span GetEUI48() const { return Span((const void*)&EUI48L, 6); }
    //! Gets the 64-bit Unique Identifier of the device
    Span GetUniqueID() const { return Span((const void*)&UNIQUEL, 8); }

    //! Gets the size of FLASH in bytes
    uint32_t FlashSize() const { return ((this->MSIZE & _DEVINFO_MSIZE_FLASH_MASK) >> _DEVINFO_MSIZE_FLASH_SHIFT) << 10; }
    //! Gets the size of RAM in bytes
    uint32_t RamSize() const { return ((this->MSIZE & _DEVINFO_MSIZE_SRAM_MASK) >> _DEVINFO_MSIZE_SRAM_SHIFT) << 10; }
};

