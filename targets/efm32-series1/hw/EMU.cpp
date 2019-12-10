/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/EMU.cpp
 */

#include <hw/EMU.h>

#include <em_emu.h>

#define MYDBG(...)  DBGCL("EMU", __VA_ARGS__)

void _EMU::Configure()
{
#if EFM32_USE_DCDC
    static const EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
    EMU_DCDCInit(&dcdcInit);
#elif EFM32_BYPASS_DCDC
    EMU->DCDCCTRL = EMU_DCDCCTRL_DCDCMODE_BYPASS;
    EMU->PWRCTRL = EMU_PWRCTRL_ANASW_AVDD | EMU_PWRCTRL_REGPWRSEL_DVDD;
#endif
}

#if defined(Ckernel) && defined(_EMU_R5VOUTLEVEL_OUTLEVEL_MASK)

async(_EMU::SetR5VOutputLevel, int mv100)
async_def()
{
    MYDBG("Current R5V output: %.1qV", R5VOutputLevel());
    R5VOutputLevel(mv100);
    await_mask(IF, EMU_IF_R5VVSINT, EMU_IF_R5VVSINT);
    MYDBG("New R5V output: %.1qV", R5VOutputLevel());
}
async_end

#endif
