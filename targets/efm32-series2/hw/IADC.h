/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series2/hw/IADC.h
 */

#pragma once

#include <base/base.h>

#include <hw/GPIO.h>

#undef IADC0
#define IADC0    CM_PERIPHERAL(IADC, IADC0_BASE)

class IADC : public IADC_TypeDef
{
public:
    enum Flags
    {
        WarmupNormal = IADC_CTRL_WARMUPMODE_NORMAL,
        WarmupStandby = IADC_CTRL_WARMUPMODE_KEEPINSTANDBY,
        WarmupKeepWarm = IADC_CTRL_WARMUPMODE_KEEPWARM,

        FlagMask = _IADC_CTRL_WARMUPMODE_MASK,
    };

    enum ModeFlags
    {
        ForceUnipolar = IADC_CFG_TWOSCOMPL_FORCEUNIPOLAR,
        ForceBipolar = IADC_CFG_TWOSCOMPL_FORCEBIPOLAR,

        Reference1V21 = IADC_CFG_REFSEL_VBGR,
        ReferenceExtSingle = IADC_CFG_REFSEL_VREF,
        ReferenceAVDD = IADC_CFG_REFSEL_VDDX,
        ReferenceAVDDp8 = IADC_CFG_REFSEL_VDDX0P8BUF,
    };

    //! Gets the index of the peripheral
    ALWAYS_INLINE int Index() const { return ((unsigned)this >> 10) & 0xF; }

    void EnableClock() { CMU->EnableIADC(Index()); }
    void Configure(Flags flags)
    {
        CTRL = flags | 40 << _IADC_CTRL_TIMEBASE_SHIFT;
        EN = IADC_EN_EN;
    }

    async(MeasureSingle, APORTX pos, APORTY neg = APORTY::VSS) { return async_forward(_MeasureSingle, pos << 8 | neg << 16); }
    async(WaitForScan);

    constexpr static float Multiplier(float range) { return range / 65535; }

private:
    async(_MeasureSingle, uint32_t singleCtrl);

    static void EnableIRQ();
    void TryDisableIRQ();
};

DEFINE_FLAG_ENUM(IADC::Flags);
DEFINE_FLAG_ENUM(IADC::ModeFlags);
