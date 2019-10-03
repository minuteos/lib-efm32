/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-rtcc/hw/RTCC.cpp
 */

#include <hw/RTCC.h>

void _RTCC::Configure()
{
    bool init = !CMU->RTCCEnabled();
    CMU->EnableRTCC();

    Setup(Enable | Prescale);

    Cortex_SetIRQWakeup(RTCC_IRQn);
    NVIC_EnableIRQ(RTCC_IRQn);

    if (init)
    {
        CNT = 0;
        memset(RET, 0, sizeof(RET));
    }

    DBGCL("RTCC", "COMBCNT: %X, CNT: %d, RESC: %08X", Ticks(), CNT, Time(), RMU->RSTCAUSE);
}

void _RTCC::SetupWake(mono_t atTicks)
{
    EFM32_BITCLR(IEN, BIT(1 + WakeChannel));
    CC[WakeChannel].CTRL = 0;
    IFC = BIT(1 + WakeChannel);
    CC[WakeChannel].CCV = atTicks;
    CC[WakeChannel].CTRL = ChannelModeCompare | ChannelComparePrescaler;
    EFM32_BITSET(IEN, BIT(1 + WakeChannel));
}
