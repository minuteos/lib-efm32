/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/WDOG.cpp
 */

#include <hw/WDOG.h>

void WDOG::Configure(unsigned timeout, bool lock)
{
#ifdef WDOG_CTRL_EN
    CTRL = WDOG_CTRL_EN |
        (WDOG_CTRL_LOCK * lock) |           // optional lock
        (2 << _WDOG_CTRL_WARNSEL_SHIFT) |   // wakeup interrupt at 50%
#if DEBUG
        WDOG_CTRL_WDOGRSTDIS |              // disable reset, will be logged instead
#endif
        (TimeoutToPeriod(timeout) << _WDOG_CTRL_PERSEL_SHIFT);
#else
    CMU->EnableWDOG(0);
    CMU->WDOG0CLKCTRL = CMU_WDOG0CLKCTRL_CLKSEL_ULFRCO;
    EN = 0;
    CFG = WDOG_CFG_WARNSEL_SEL2 |
#if DEBUG
        WDOG_CFG_WDOGRSTDIS |               // disable reset, will be logged instead
#endif
        (TimeoutToPeriod(timeout) << _WDOG_CFG_PERSEL_SHIFT);

    EN = WDOG_EN_EN;
    if (lock) LOCK = WDOG_LOCK_LOCKKEY_LOCK;
#endif

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
    EFM32_SetIRQClearingHandler(irq, EFM32_IFC(this));
#endif
    NVIC_EnableIRQ(irq);
}

#if DEBUG
void WDOG::IRQHandler()
{
    if (EFM32_IFC_READ(this) & WDOG_IF_TOUT)
    {
        DBGCL("WATCHDOG", "TIMEOUT!!!");
        ASSERT(false);
    }
}
#endif
