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
#if DEBUG
        WDOG_CTRL_WDOGRSTDIS |              // disable reset, will be logged instead
#endif
        (TimeoutToPeriod(timeout) << _WDOG_CTRL_PERSEL_SHIFT);
    Sync();
#if DEBUG
    IEN = WDOG_IEN_WARN | WDOG_IEN_TOUT;
#else
    IEN = WDOG_IEN_WARN;
#endif

    auto irq = IRQn(WDOG0_IRQn + Index());
#if DEBUG
    Cortex_SetIRQHandler(irq, GetDelegate(this, &WDOG::IRQHandler));
#else
    EFM32_SetIRQClearingHandler(irq, IFC);
#endif
    NVIC_EnableIRQ(irq);
}

#if DEBUG
void WDOG::IRQHandler()
{
    if (IFC & WDOG_IFC_TOUT)
    {
        DBGCL("WATCHDOG", "TIMEOUT!!!");
        ASSERT(false);
    }
}
#endif
