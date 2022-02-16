/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-rtcc/hw/RTCC.h
 *
 * Convenience wrapper around the EFM32 RTCC peripheral
 */

#pragma once

#include <base/base.h>

#ifdef Ckernel
#include <kernel/async.h>
#endif

#include <hw/CMU.h>

#undef RTCC
#define RTCC	CM_PERIPHERAL(_RTCC, RTCC_BASE)

class _RTCC : public RTCC_TypeDef
{
public:
    enum ChannelFlags
    {
        ChannelModeCapture = RTCC_CC_CTRL_MODE_INPUTCAPTURE,
        ChannelModeCompare = RTCC_CC_CTRL_MODE_OUTPUTCOMPARE,

        ChannelOutputPulse = RTCC_CC_CTRL_CMOA_PULSE,
        ChannelOutputToggle = RTCC_CC_CTRL_CMOA_TOGGLE,
        ChannelOutputClear = RTCC_CC_CTRL_CMOA_CLEAR,
        ChannelOutputSet = RTCC_CC_CTRL_CMOA_SET,

        ChannelInputRising = RTCC_CC_CTRL_ICEDGE_RISING,
        ChannelInputFalling = RTCC_CC_CTRL_ICEDGE_FALLING,
        ChannelInputBoth = RTCC_CC_CTRL_ICEDGE_BOTH,
        ChannelInputNone = RTCC_CC_CTRL_ICEDGE_NONE,

        ChannelCompareCounter = RTCC_CC_CTRL_COMPBASE_CNT,
        ChannelComparePrescaler = RTCC_CC_CTRL_COMPBASE_PRECNT,

#ifdef _RTCC_CC_CTRL_PRSSEL_SHIFT
        ChannelPRSOffset = _RTCC_CC_CTRL_PRSSEL_SHIFT,
#endif

#ifdef _RTCC_CC_CTRL_COMPMASK_SHIFT
        ChannelCompareMaskOffset = _RTCC_CC_CTRL_COMPMASK_SHIFT,
#endif
    };

    enum
    {
        WakeChannel = 2,
    };

    void Configure();

    //! Gets the overflowing tick counter
    uint32_t Ticks() { return COMBCNT; }

    //! Gets the real time, if set previously
    uint32_t Time(bool offset = true) { return CNT + (offset ? TimeOffset() : 0); }
    //! Gets the real time, with fractional part in the lower 32-bits
    uint64_t Timestamp(bool offset = true);

    //! Sets the real time
    void SetTime(uint32_t time) { TimeOffset() = nonzero(time - CNT); }
    //! Configures the specified channel for input capture
    void SetupCapture(unsigned channel, unsigned prsChannel, ChannelFlags flags)
    {
        ASSERT(channel < countof(CC));
#ifdef _SILICON_LABS_32B_SERIES_2
        PRS->CONSUMER_RTCC_CC0 = prsChannel;
        CC[channel].CTRL = flags | ChannelModeCapture;
#else
        CC[channel].CTRL = flags | ChannelModeCapture | ChannelFlags(prsChannel << ChannelPRSOffset);
#endif
    }
    //! Reads the captured value for the specified channel and clears the corresponding interrupt
    uint32_t ReadCaptured(unsigned channel) { EFM32_IFC(this) = InterruptMask(channel); return InputCapture(channel); }

    void SetupWake(uint32_t at);
    void DisableWake() { InterruptDisable(WakeChannel); }

    void InterruptEnable(unsigned channel) { EFM32_BITSET_REG(IEN, InterruptMask(channel)); }
    void InterruptDisable(unsigned channel) { EFM32_BITCLR_REG(IEN, InterruptMask(channel)); }
    void InterruptClear(unsigned channel) { EFM32_IFC(this) = InterruptMask(channel); }
    bool InterruptActive(unsigned channel) { return IF & InterruptMask(channel); }

    volatile uint32_t& TimeOffset() { return BackupRegister(0); }

#ifdef Ckernel
    async(WaitFor, unsigned channel);
#endif

private:
#ifdef _SILICON_LABS_32B_SERIES_2
    uint32_t InputCapture(unsigned channel) { return CC[channel].ICVALUE; }
    volatile uint32_t& OutputCompare(unsigned channel) { return CC[channel].OCVALUE; }
    static volatile uint32_t& BackupRegister(unsigned index) { return BURAM->RET[index].REG; }
    static constexpr uint32_t InterruptMask(unsigned channel) { return RTCC_IF_CC0 << (channel << 1); }
#else
    uint32_t InputCapture(unsigned channel) { return CC[channel].CCV; }
    volatile uint32_t& OutputCompare(unsigned channel) { return CC[channel].CCV; }
    volatile uint32_t& BackupRegister(unsigned index) { return RET[index].REG; }
    static constexpr uint32_t InterruptMask(unsigned channel) { return RTCC_IF_CC0 << channel; }
#endif
};

DEFINE_FLAG_ENUM(_RTCC::ChannelFlags);
