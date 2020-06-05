/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/WDOG.cpp
 */

#include <hw/WDOG.h>

void WDOG::Configure(unsigned timeout, bool lock)
{
    CTRL = WDOG_CTRL_EN |
        (WDOG_CTRL_LOCK * lock) |           // optional lock
        (2 << _WDOG_CTRL_WARNSEL_SHIFT) |   // wakeup interrupt at 50%
        (TimeoutToPeriod(timeout) << _WDOG_CTRL_PERSEL_SHIFT);
    Sync();
    IEN = WDOG_IEN_WARN;

    auto irq = IRQn(WDOG0_IRQn + Index());
    EFM32_SetIRQClearingHandler(irq, IFC);
    NVIC_EnableIRQ(irq);
}
