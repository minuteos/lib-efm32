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
#ifdef _RTCC_CTRL_MASK
    bool init = !CMU->RTCCEnabled();
    CMU->EnableRTCC();

    CTRL = RTCC_CTRL_ENABLE | RTCC_CTRL_CNTPRESC_DIV32768;
#else
    CMU->EnableRTCC();
    bool init = false;
    if (CFG != RTCC_CFG_CNTPRESC_DIV32768)
    {
        init = true;
        EN = 0;
        CNT = 0;
        CFG = RTCC_CFG_CNTPRESC_DIV32768;
    }
    EN = RTCC_EN_EN;
    CMD = RTCC_CMD_START;
#endif

    Cortex_SetIRQWakeup(RTCC_IRQn);

    if (init)
    {
        memset((void*)&BackupRegister(0), 0, 128);
    }

    DBGCL("RTCC", "COMBCNT: %X, CNT: %d", Ticks(), CNT);
}

void _RTCC::SetupWake(mono_t atTicks)
{
    CC[WakeChannel].CTRL = 0;
    InterruptDisable(WakeChannel);
    InterruptClear(WakeChannel);
    SCB->EnableWake(RTCC_IRQn); // also clears any pending request
    OutputCompare(WakeChannel) = atTicks;
    CC[WakeChannel].CTRL = ChannelModeCompare | ChannelComparePrescaler;
    InterruptEnable(WakeChannel);
    // we might have already crossed the wake moment
    if (OVF_GE(COMBCNT, atTicks))
    {
        SCB->TriggerWake(RTCC_IRQn);
    }
}

uint64_t _RTCC::Timestamp()
{
    uint32_t time = CNT, comb = COMBCNT;
    if (GETMASK(time, 17) != comb >> 15)
    {
        // if combcnt has overflowed between the two reads, time has to be exactly one higher
        ASSERT(false);
        time++;
        ASSERT(GETMASK(time, 17) == comb >> 15);
    }
    time += TimeOffset();
    return pack64(comb << 17, time);
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
