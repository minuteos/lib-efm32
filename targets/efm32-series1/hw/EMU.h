/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/EMU.h
 */

#pragma once

#include <base/base.h>

#ifdef Ckernel
#include <kernel/kernel.h>
#endif

#undef EMU
#define EMU	CM_PERIPHERAL(_EMU, EMU_BASE)

class _EMU : public EMU_TypeDef
{
public:
    enum GPIORetain
    {
        Off = EMU_EM4CTRL_EM4IORETMODE_DISABLE,
        UntilWakeup = EMU_EM4CTRL_EM4IORETMODE_EM4EXIT,
        UntilUnlatch = EMU_EM4CTRL_EM4IORETMODE_SWUNLATCH,
    };

    void Configure();

    void Unlatch() { CMD = EMU_CMD_EM4UNLATCH; }

    void Hibernate(unsigned ms = 0, GPIORetain retain = GPIORetain::UntilUnlatch) { EnterEM4(ms, EMU_EM4CTRL_EM4STATE_EM4H | retain); }
    void Shutoff(unsigned ms = 0, GPIORetain retain = GPIORetain::UntilUnlatch) { EnterEM4(ms, EMU_EM4CTRL_EM4STATE_EM4S | retain); }

    void EnterEM4(unsigned cryoWakeup, uint32_t em4ctrl);

#ifdef _EMU_R5VOUTLEVEL_OUTLEVEL_MASK
    int R5VOutputLevel() { return 23 + ((R5VOUTLEVEL & _EMU_R5VOUTLEVEL_OUTLEVEL_MASK) >> _EMU_R5VOUTLEVEL_OUTLEVEL_SHIFT); }
    void R5VOutputLevel(int mv100) { MODMASK_SAFE(R5VOUTLEVEL, _EMU_R5VOUTLEVEL_OUTLEVEL_MASK, (mv100 - 23) << _EMU_R5VOUTLEVEL_OUTLEVEL_SHIFT); }
#ifdef Ckernel
    async(SetR5VOutputLevel, int mv100);
#endif
#endif
};
