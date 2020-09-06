/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series2/hw/EMU.cpp
 */

#include <hw/EMU.h>

#include <em_emu.h>

#define MYDBG(...)  DBGCL("EMU", __VA_ARGS__)

void _EMU::Configure()
{
#if EFM32_USE_DCDC
    static const EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
    EMU_DCDCInit(&dcdcInit);
#endif
}
