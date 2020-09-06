/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series2/hw/CMU.cpp
 */

#include <hw/CMU.h>

#include <em_cmu.h>
#include <algorithm>

#if CMU_DIAGNOSTICS
#define MYDIAG(...)   DBGCL("CMU", __VA_ARGS__)
#else
#define MYDIAG(...)
#endif

void _CMU::Configure()
{
#if EFM32_HFXO_FREQUENCY
    CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;
    CMU_HFXOInit(&hfxoInit);
#endif

    // first configure the low frequency oscillator
#if !EFM32_LFXO_FREQUENCY
    // crystal not available, use the LFRCO which starts up quickly so we can wait for it
    CLKEN0_SET = CMU_CLKEN0_LFRCO;
    RTCCCLKCTRL = CMU_RTCCCLKCTRL_CLKSEL_LFRCO;
#else
    CLKEN0_SET = CMU_CLKEN0_LFXO;
    if (LFXO->STATUS & LFXO_STATUS_RDY)
    {
        // LFXO is running at startup
        MYDIAG("LFXO running...");
        RTCCCLKCTRL = CMU_RTCCCLKCTRL_CLKSEL_LFXO;
    }
    else
    {
        CLKEN0_SET = CMU_CLKEN0_LFRCO;
        LFXO->CTRL = LFXO_CTRL_DISONDEMAND;
        LFXO->CAL = std::min(unsigned((EFM32_LFXO_LOAD_CAPACITANCE * 2 - 4) * 4), 0x4Fu) << _LFXO_CAL_CAPTUNE_SHIFT |
            std::min(unsigned(EFM32_LFXO_GAIN), 3u) << _LFXO_CAL_GAIN_SHIFT;
        RTCCCLKCTRL = CMU_RTCCCLKCTRL_CLKSEL_LFRCO;
        // RTCCCLK will be switched to LFXO once it is stable
        Cortex_SetIRQHandler(LFXO_IRQn, GetDelegate(this, &_CMU::LFXOReadyHandler));
        LFXO->IEN_SET = LFXO_IEN_RDY;
        LFXO->CTRL = LFXO_CTRL_FORCEEN;
        NVIC_EnableIRQ(LFXO_IRQn);
    }
#endif

#ifdef EFM32_HFXO_FREQUENCY
    // SYSCLK will be switched to HFXO once it is stable
    Cortex_SetIRQHandler(HFXO0_IRQn, GetDelegate(this, &_CMU::HFXOReadyHandler));
    HFXO0->IEN_SET = HFXO_IEN_RDY;
    NVIC_EnableIRQ(HFXO0_IRQn);
    HFXO0->CTRL = HFXO_CTRL_FORCEEN;
#elif defined(EFM32_HFRCO_FREQUENCY)
    CMU_HFRCOBandSet(EFM32_HFRCO_FREQUENCY);
#endif

}

void _CMU::LFXOReadyHandler()
{
    LFXO->IF_CLR = LFXO_IF_RDY;
    // switch LFXO
    RTCCCLKCTRL = CMU_RTCCCLKCTRL_CLKSEL_LFXO;
    EM23GRPACLKCTRL = CMU_EM23GRPACLKCTRL_CLKSEL_LFXO;
    EM4GRPACLKCTRL = CMU_EM4GRPACLKCTRL_CLKSEL_LFXO;
}

void _CMU::HFXOReadyHandler()
{
    HFXO0->IF_CLR = HFXO_IF_RDY;
    // switch to HFXO
    SYSCLKCTRL = CMU_SYSCLKCTRL_CLKSEL_HFXO;
    EM01GRPACLKCTRL = CMU_EM01GRPACLKCTRL_CLKSEL_HFXO;
    EM01GRPBCLKCTRL = CMU_EM01GRPBCLKCTRL_CLKSEL_HFXO;
}
