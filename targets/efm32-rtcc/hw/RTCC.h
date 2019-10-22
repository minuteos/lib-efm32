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

#include <hw/CMU.h>

#undef RTCC
#define RTCC	CM_PERIPHERAL(_RTCC, RTCC_BASE)

class _RTCC : public RTCC_TypeDef
{
public:
    enum Flags
    {
        Enable = BIT(0),
        Debug = BIT(2),
        Prescale = 15 << 8,		// we allow only full prescaling to seconds, which is the only one that makes sense, since COMBCNT becomes a 32k counter while CNT is the number of seconds in this configuration
        FailureDetection = BIT(15),
    };

    enum ChannelFlags
    {
        ChannelModeCapture = 1,
        ChannelModeCompare = 2,

        ChannelOutputPulse = 0 << 2,
        ChannelOutputToggle = 1 << 2,
        ChannelOutputClear = 2 << 2,
        ChannelOutputSet = 3 << 2,

        ChannelInputRising = 0 << 4,
        ChannelInputFalling = 1 << 4,
        ChannelInputBoth = 2 << 4,
        ChannelInputNone = 3 << 4,

        ChannelPRSOffset = 6,

        ChannelCompareCounter = 0,
        ChannelComparePrescaler = BIT(11),

        ChannelCompareMaskOffset = 12,
    };

    enum
    {
        WakeChannel = 2,
    };

    void Configure();

    //! Gets the overflowing tick counter
    uint32_t Ticks() { return COMBCNT; }

    //! Gets the real time, if set previously
    uint32_t Time() { return CNT + TimeOffset(); }
    //! Sets the real time
    void SetTime(uint32_t time) { TimeOffset() = time - CNT; }

    void SetupWake(uint32_t at);
    void DisableWake() { InterruptDisable(WakeChannel); }

    void InterruptEnable(unsigned channel) { IEN |= RTCC_IEN_CC0 << channel; }
    void InterruptDisable(unsigned channel) { IEN &= ~(RTCC_IEN_CC0 << channel); }
    void InterruptClear(unsigned channel) { IFC = RTCC_IEN_CC0 << channel; }
    bool InterruptActive(unsigned channel) { return IF & (RTCC_IEN_CC0 << channel); }

private:
    // RTC must not be configured by application
    void Setup(Flags flags) { CTRL = flags; }

    volatile uint32_t& TimeOffset() { return RET[0].REG; }
};

DEFINE_FLAG_ENUM(_RTCC::Flags);
