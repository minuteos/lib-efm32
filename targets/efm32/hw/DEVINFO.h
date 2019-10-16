/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/DEVINFO.h
 */

#pragma once

#include <base/base.h>

#undef DEVINFO
#define DEVINFO    CM_PERIPHERAL(_DEVINFO, DEVINFO_BASE)

class _DEVINFO : public DEVINFO_TypeDef
{
public:
    uint32_t FlashSize() const { return ((this->MSIZE & _DEVINFO_MSIZE_FLASH_MASK) >> _DEVINFO_MSIZE_FLASH_SHIFT) << 10; }
    uint32_t RamSize() const { return ((this->MSIZE & _DEVINFO_MSIZE_SRAM_MASK) >> _DEVINFO_MSIZE_SRAM_SHIFT) << 10; }
};

