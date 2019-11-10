/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/ADC.h
 */

#pragma once

#include <kernel/kernel.h>

#include <hw/CMU.h>
#include <hw/PRS.h>

#undef ADC0
#define ADC0	CM_PERIPHERAL(ADC, ADC0_BASE)

class ADC : public ADC_TypeDef
{
public:
    enum Flags
    {
        WarmupNormal = ADC_CTRL_WARMUPMODE_NORMAL,
        WarmupStandby = ADC_CTRL_WARMUPMODE_KEEPINSTANDBY,
        WarmupSlowAcquisition = ADC_CTRL_WARMUPMODE_KEEPINSLOWACC,
        WarmupKeepWarm = ADC_CTRL_WARMUPMODE_KEEPADCWARM,

        SingleDMAWakeup = ADC_CTRL_SINGLEDMAWU,
        ScanDMAWakeup = ADC_CTRL_SCANDMAWU,

        SingleTailgating = ADC_CTRL_TAILGATE,

        ClockSync = ADC_CTRL_ADCCLKMODE_SYNC,
        ClockAsyncAsNeeded = ADC_CTRL_ADCCLKMODE_ASYNC | ADC_CTRL_ASYNCCLKEN_ASNEEDED,
        ClockAsyncAlwaysOn = ADC_CTRL_ADCCLKMODE_ASYNC | ADC_CTRL_ASYNCCLKEN_ALWAYSON,

        OptimizeSettlingTime = ADC_CTRL_CHCONMODE_MAXSETTLE,
        OptimizeResponse = ADC_CTRL_CHCONMODE_MAXRESP,

        FlagMask = _ADC_CTRL_WARMUPMODE_MASK |
            _ADC_CTRL_SINGLEDMAWU_MASK | _ADC_CTRL_SCANDMAWU_MASK |
            _ADC_CTRL_TAILGATE_MASK |
            _ADC_CTRL_ADCCLKMODE_MASK | _ADC_CTRL_ASYNCCLKEN_MASK |
            _ADC_CTRL_CHCONMODE_MASK,
    };

    enum ModeFlags
    {
        RepeatOff = ADC_SINGLECTRL_REP_DEFAULT,
        RepeatOn = ADC_SINGLECTRL_REP,

        SingleEnded = ADC_SINGLECTRL_DIFF_DEFAULT,
        Differential = ADC_SINGLECTRL_DIFF,

        AdjustRight = ADC_SINGLECTRL_ADJ_RIGHT,
        AdjustLeft = ADC_SINGLECTRL_ADJ_LEFT,

        Resolution12Bit = ADC_SINGLECTRL_RES_12BIT,
        Resolution8Bit = ADC_SINGLECTRL_RES_8BIT,
        Resolution6Bit = ADC_SINGLECTRL_RES_6BIT,
        ResolutionOversampled = ADC_SINGLECTRL_RES_OVS,

        Reference1V25 = ADC_SINGLECTRL_REF_1V25,
        Reference2V5 = ADC_SINGLECTRL_REF_2V5,
        ReferenceVDD = ADC_SINGLECTRL_REF_VDD,
        Reference5V = ADC_SINGLECTRL_REF_5V,
        ReferenceExtSingle = ADC_SINGLECTRL_REF_EXTSINGLE,
        Reference2xExtDiff = ADC_SINGLECTRL_REF_2XEXTDIFF,
        Reference2xVDD = ADC_SINGLECTRL_REF_2XVDD,
        ReferenceCustom = ADC_SINGLECTRL_REF_CONF,

        Acquisition1C = ADC_SINGLECTRL_AT_1CYCLE,
        Acquisition2C = ADC_SINGLECTRL_AT_2CYCLES,
        Acquisition3C = ADC_SINGLECTRL_AT_3CYCLES,
        Acquisition4C = ADC_SINGLECTRL_AT_4CYCLES,
        Acquisition8C = ADC_SINGLECTRL_AT_8CYCLES,
        Acquisition16C = ADC_SINGLECTRL_AT_16CYCLES,
        Acquisition32C = ADC_SINGLECTRL_AT_32CYCLES,
        Acquisition64C = ADC_SINGLECTRL_AT_64CYCLES,
        Acquisition128C = ADC_SINGLECTRL_AT_128CYCLES,
        Acquisition256C = ADC_SINGLECTRL_AT_256CYCLES,
        AcquisitionMask = _ADC_SINGLECTRL_AT_MASK,
    };

    int Index() const { return 0; }

    void EnableClock() { CMU->EnableADC(Index()); }
    void DisableClock() { CMU->DisableADC(Index()); }

    void Setup(Flags flags) { MODMASK(CTRL, FlagMask, flags); }

    unsigned OversampleRate() const { return 2 << ((CTRL & _ADC_CTRL_OVSRSEL_MASK) >> _ADC_CTRL_OVSRSEL_SHIFT); }
    void OversampleRate(unsigned rate) { MODMASK_SAFE(CTRL, _ADC_CTRL_OVSRSEL_MASK << 24, (30 - __CLZ(rate)) << _ADC_CTRL_OVSRSEL_SHIFT); }

    uint32_t Timebase() const { return ((CTRL & _ADC_CTRL_TIMEBASE_MASK) >> _ADC_CTRL_TIMEBASE_SHIFT) + 1; }
    void Timebase(uint32_t clocks) { MODMASK_SAFE(CTRL, _ADC_CTRL_TIMEBASE_MASK, (clocks - 1) << 16); }

    uint32_t Prescaler() const { return (CTRL & _ADC_CTRL_PRESC_MASK) >> _ADC_CTRL_PRESC_SHIFT; }
    void Prescaler(uint32_t value) { MODMASK_SAFE(CTRL, _ADC_CTRL_PRESC_MASK, value << _ADC_CTRL_PRESC_SHIFT); }

    void SingleStart() { CMD = ADC_CMD_SINGLESTART; }
    void SingleStop() { CMD = ADC_CMD_SINGLESTOP; }
    void ScanStart() { CMD = ADC_CMD_SCANSTART; }
    void ScanStop() { CMD = ADC_CMD_SCANSTOP; }

    bool SingleActive() const { return STATUS & ADC_STATUS_SINGLEACT; }
    bool ScanActive() const { return STATUS & ADC_STATUS_SCANACT; }
    bool Active() const { return STATUS & (ADC_STATUS_SINGLEACT | ADC_STATUS_SCANACT); }

    bool SingleReferenceWarmedUp() const { return STATUS & ADC_STATUS_SINGLEREFWARM; }
    bool ScanReferenceWarmedUp() const { return STATUS & ADC_STATUS_SCANREFWARM; }
    bool WarmedUp() const { return STATUS & ADC_STATUS_WARM; }

    bool ErrorBusConflict() const { return STATUS & ADC_STATUS_PROGERR_BUSCONF; }
    bool ErrorInvalidNegative() const { return STATUS & ADC_STATUS_PROGERR_NEGSELCONF; }

    bool SingleValid() const { return STATUS & ADC_STATUS_SINGLEDV; }
    bool ScanValid() const { return STATUS & ADC_STATUS_SCANDV; }

    uint32_t SingleFifoCount() const { return SINGLEFIFOCOUNT; }
    uint32_t ScanFifoCount() const { return SCANFIFOCOUNT; }

    uint32_t SingleRead() { return SINGLEDATA; }
    uint32_t ScanRead() { return SCANDATA; }

    void SingleSetup(ModeFlags flags, APORTX pos, APORTY neg = APORTY::VSS) { SINGLECTRL = flags | pos << 8 | neg << 16; }
    void ScanSetup(ModeFlags flags, const APORT* ports, size_t count);
    template<typename... Ports> ALWAYS_INLINE void ScanSetup(ModeFlags flags, Ports... ports) { const APORTX arr[] = { ports... }; ScanSetup(flags, (const APORT*)arr, sizeof...(ports)); }

    void ConfigureSinglePRSTrigger(PRSChannelHandle prs, bool timed = false)
    {
        MODMASK(SINGLECTRLX, MASK(5) << 16, (prs << 17) | (timed * BIT(16)));
    }
    void EnableSinglePRSTrigger(bool enable = true) { MODBIT(SINGLECTRL, 29, enable); }

    void ConfigureScanPRSTrigger(PRSChannelHandle prs, bool timed = false)
    {
        MODMASK(SCANCTRLX, MASK(5) << 16, (prs << 17) | (timed * BIT(16)));
    }
    void EnableScanPRSTrigger(bool enable = true) { MODBIT(SCANCTRL, 29, enable); }

    void DisableMaster(APORT port) { APORTMASTERDIS |= port.BusMask(); }
    void EnableMaster(APORT port) { APORTMASTERDIS &= ~port.BusMask(); }

    volatile const void* SinglePtr() { return &SINGLEDATA; }
    volatile const void* ScanPtr() { return &SCANDATA; }

    async(MeasureSingle, ModeFlags flags, APORTX pos, APORTY neg = APORTY::VSS) { return async_forward(_MeasureSingle, flags | pos << 8 | neg << 16); }

    constexpr static float Multiplier(float range) { return range / 65535; }

private:
    async(_MeasureSingle, uint32_t singleCtrl);
};

DEFINE_FLAG_ENUM(ADC::Flags);
DEFINE_FLAG_ENUM(ADC::ModeFlags);
