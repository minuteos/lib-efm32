/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/WDOG.h
 */

#pragma once

#include <base/base.h>

#include <hw/CMU.h>

#include <algorithm>

#undef WDOG0
#define WDOG0   CM_PERIPHERAL(WDOG, WDOG0_BASE)

#ifdef WDOG1_BASE
#undef WDOG1
#define WDOG1   CM_PERIPHERAL(WDOG, WDOG1_BASE)
#endif

class WDOG : public WDOG_TypeDef
{
public:
    //! Gets the index of the peripheral
    unsigned Index() const { return ((unsigned)this >> 10) & 0x3; }

    //! Configure watchdog with the specified timeout, optionally locking the configuration
    void Configure(unsigned timeout, bool lock = false);
    //! Check is the watchdog is enabled
    bool Enabled() const { return CTRL & WDOG_CTRL_EN; }
    //! Disable watchdog
    void Disable() { if (Enabled()) { Sync(); CTRL = 0; } }
    //! Sync pending commands
    void Sync() { while (SYNCBUSY); }

    //! Hit watchdog, safe to call repeatedly
    ALWAYS_INLINE void Hit()
    {
        if (!(SYNCBUSY & WDOG_SYNCBUSY_CMD))
        {
            CMD = WDOG_CMD_CLEAR;
        }
    }

private:
    //! calculate watchdog period from timeout
    //! actual timeout = 2^(period + 3), make it always higher than requested
    static constexpr unsigned TimeoutToPeriod(unsigned timeout)
    {
        return std::min(15, 29 - __builtin_clz(timeout));
    }

#if DEBUG
    void IRQHandler();
#endif
};
