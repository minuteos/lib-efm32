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
    //! Configure watchdog with the specified timeout, optionally locking the configuration
    void Configure(unsigned timeout, bool lock = false)
    {
        CMU->EnableLE();
        CTRL = WDOG_CTRL_EN | (WDOG_CTRL_LOCK * lock) | TimeoutToPeriod(timeout) << _WDOG_CTRL_PERSEL_SHIFT;
    }
    //! Disable watchdog
    void Disable() { CTRL = 0; }

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
};
