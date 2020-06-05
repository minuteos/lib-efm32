/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-rtcc/hw/RTCC.cpp
 */

#include <hw/RTCC.h>
#include <hw/SCB.h>

#include <kernel/kernel.h>

void _RTCC::Configure()
{
    bool init = !CMU->RTCCEnabled();
    CMU->EnableRTCC();

    Setup(Enable | Prescale);

    Cortex_SetIRQWakeup(RTCC_IRQn);

    if (init)
    {
        CNT = 0;
        memset(RET, 0, sizeof(RET));
    }

    DBGCL("RTCC", "COMBCNT: %X, CNT: %d", Ticks(), CNT);
}

void _RTCC::SetupWake(mono_t atTicks)
{
    CC[WakeChannel].CTRL = 0;
    InterruptDisable(WakeChannel);
    InterruptClear(WakeChannel);
    SCB->EnableWake(RTCC_IRQn); // also clears any pending request
    CC[WakeChannel].CCV = atTicks;
    CC[WakeChannel].CTRL = ChannelModeCompare | ChannelComparePrescaler;
    InterruptEnable(WakeChannel);
    // we might have already crossed the wake moment
    if (OVF_GE(COMBCNT, atTicks))
    {
        SCB->TriggerWake(RTCC_IRQn);
    }
}

#ifdef Ckernel

async(_RTCC::WaitFor, unsigned channel)
async_def()
{
    if (!InterruptActive(channel))
    {
        // first call
        InterruptEnable(channel);
        await_mask(IF, RTCC_IF_CC0 << channel, RTCC_IF_CC0 << channel);
        InterruptDisable(channel);
    }

    InterruptClear(channel);
}
async_end

#endif
