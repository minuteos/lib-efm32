/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/CMU.cpp
 */

#include <hw/CMU.h>

#include <em_cmu.h>

#if CMU_DIAGNOSTICS
#define MYDIAG(...)   DBGCL("CMU", __VA_ARGS__)
#else
#define MYDIAG(...)
#endif

void _CMU::Configure()
{
    MYDIAG("Init STATUS: %08X", STATUS);

#if EFM32_LFXO_FREQUENCY
    CMU_OscillatorEnable(cmuOsc_LFXO, true, false);
#endif
#if EFM32_HFXO_FREQUENCY
    CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;
    hfxoInit.ctuneSteadyState = 282;
    HFXOCTRL &= ~CMU_HFXOCTRL_AUTOSTARTEM0EM1;
    CMU_HFXOInit(&hfxoInit);
    CMU_OscillatorEnable(cmuOsc_HFXO, true, EFM32_WAIT_FOR_HFXO);
    // autostart HFXO whenever MCU is running
    HFXOCTRL |= CMU_HFXOCTRL_AUTOSTARTEM0EM1;
#endif

#if (EFM32_LFXO_FREQUENCY && !EFM32_WAIT_FOR_LFXO) || \
    (EFM32_HFXO_FREQUENCY && !EFM32_WAIT_FOR_HFXO)
    // configure CMU interrupt for when the LFXO starts
    Cortex_SetIRQHandler(CMU_IRQn, GetDelegate(this, &_CMU::IRQHandler));
    NVIC_EnableIRQ(CMU_IRQn);
#endif

    // first configure the low frequency oscillator
#if !EFM32_LFXO_FREQUENCY
    // crystal not available, use the LFRCO which starts up quickly so we can wait for it
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
    LFACLKSEL = CMU_LFACLKSEL_LFA_LFRCO;
    LFBCLKSEL = CMU_LFBCLKSEL_LFB_LFRCO;
#ifdef CMU_LFCCLKSEL_LFC_LFRCO
    LFCCLKSEL = CMU_LFCCLKSEL_LFC_LFRCO;
#endif
    LFECLKSEL = CMU_LFECLKSEL_LFE_LFRCO;
#else
#if EFM32_WAIT_FOR_LFXO
    // LFXO must be running before init will continue
    if (!LFXOReady())
    {
        MYDIAG("Waiting for LFXO...");
        while (!LFXOReady());
    }
#endif

    if (LFXOReady())
    {
        // LFXO is running
        MYDIAG("LFXO running...");
        LFACLKSEL = CMU_LFACLKSEL_LFA_LFXO;
        LFBCLKSEL = CMU_LFBCLKSEL_LFB_LFXO;
#ifdef CMU_LFCCLKSEL_LFC_LFXO
        LFCCLKSEL = CMU_LFCCLKSEL_LFC_LFXO;
#endif
        LFECLKSEL = CMU_LFECLKSEL_LFE_LFXO;
    }
#if !EFM32_WAIT_FOR_LFXO
    else
    {
        CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
        LFACLKSEL = CMU_LFACLKSEL_LFA_LFRCO;
        LFBCLKSEL = CMU_LFBCLKSEL_LFB_LFRCO;
#ifdef CMU_LFCCLKSEL_LFC_LFRCO
        LFCCLKSEL = CMU_LFCCLKSEL_LFC_LFRCO;
#endif
        LFECLKSEL = CMU_LFECLKSEL_LFE_LFRCO;
        // LFxCLKSEL will be switched to LFXO once it is stable
        EFM32_BITSET(IEN, CMU_IEN_LFXORDY);
    }
#endif
#endif

#ifdef EFM32_HFXO_FREQUENCY
    // configure HFXO
    CMU_UpdateWaitStates(EFM32_HFXO_FREQUENCY, 0);
#if EFM32_WAIT_FOR_HFXO
    // wait for HFXO to stabilize
    while (!HFXOReady());
    HFCLKSEL = CMU_HFCLKSEL_HF_HFXO;
#else
    EFM32_BITSET(IEN, CMU_IEN_HFXORDY);
#endif
#elif defined(EFM32_USHFRCO_HFCLK)
    EnableUSHFRCO();
    USHFRCOCTRL = DEVINFO->USHFRCOCAL13;
    while (!USHFRCOReady());
    CMU_UpdateWaitStates(48000000, 0);
    HFCLKSEL = CMU_HFCLKSEL_HF_USHFRCO;
#elif defined(EFM32_HFRCO_FREQUENCY)
    CMU_OscillatorEnable(cmuOsc_HFRCO);
    CMU_HFRCOBandSet(EFM32_HFRCO_FREQUENCY);
    while (!HFRCOReady());
    CMU_UpdateWaitStates(EFM32_HFRCO_FREQUENCY, 0);
    HFCLKSEL = CMU_HFCLKSEL_HF_HFRCO;
#endif
}

void _CMU::IRQHandler()
{
    UNUSED auto mask = IFC;

    MYDIAG("IRQ: %08X", mask);

#if EFM32_LFXO_FREQUENCY && !EFM32_WAIT_FOR_LFXO
    if (mask & CMU_IFC_LFXORDY)
    {
        LFACLKSEL = CMU_LFACLKSEL_LFA_LFXO;
        LFBCLKSEL = CMU_LFBCLKSEL_LFB_LFXO;
#ifdef CMU_LFCCLKSEL_LFC_LFXO
        LFCCLKSEL = CMU_LFCCLKSEL_LFC_LFXO;
#endif
        LFECLKSEL = CMU_LFECLKSEL_LFE_LFXO;
        DisableLFRCO();
    }
#endif

#if EFM32_HFXO_FREQUENCY && !EFM32_WAIT_FOR_HFXO
    if (mask & CMU_IFC_HFXORDY)
    {
        HFCLKSEL = CMU_HFCLKSEL_HF_HFXO;
        DisableHFRCO();
    }
#endif
}

#if EFM32_HFXO_FREQUENCY && EFM32_WAIT_FOR_HFXO

void _CMU::DeepSleepRestore()
{
    ASSERT(HFXOEnabled());
    // slow down HFRCO to save energy while waiting for HFXO startup
    // any pending DMA should have been processed by now, as the core wakeup takes around 10 us (i.e. hundreds of clock cycles)
    auto hfrcoPrev = HFRCOCTRL;
    HFRCOCTRL = DEVINFO->HFRCOCAL0 | CMU_HFRCOCTRL_CLKDIV_DIV4;
    while (!HFXOReady());
    HFCLKSEL = CMU_HFCLKSEL_HF_HFXO;
    DisableHFRCO();
    // restore original HFRCO setting
    HFRCOCTRL = hfrcoPrev;
}

static constexpr unsigned TO(unsigned periods) { return (periods + 11) / 12; }	// a period is defined as "at least 83 ns" which appears to be 1/12 of a microsecond (83.333...)

unsigned _CMU::DeepSleepRestoreMicroseconds()
{
    static constexpr uint16_t s_timeoutTable[] = { TO(2), TO(4), TO(16), TO(32), TO(256), TO(1024), TO(2048), TO(4096), TO(8192), TO(16384), TO(32768) };
    auto to = HFXOTIMEOUTCTRL;
    return s_timeoutTable[(to & _CMU_HFXOTIMEOUTCTRL_STARTUPTIMEOUT_MASK) >> _CMU_HFXOTIMEOUTCTRL_STARTUPTIMEOUT_SHIFT] +
        s_timeoutTable[(to & _CMU_HFXOTIMEOUTCTRL_STEADYTIMEOUT_MASK) >> _CMU_HFXOTIMEOUTCTRL_STEADYTIMEOUT_SHIFT] +
        100;	// 100 microseconds is a guesstimate for variations in HFXO startup timing
}

#endif
