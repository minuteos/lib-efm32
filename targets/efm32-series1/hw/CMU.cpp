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
    EnableLFXO();
#endif
#if EFM32_HFXO_FREQUENCY
    // autostart HFXO whenever MCU is running
    EFM32_BITSET(HFXOCTRL, CMU_HFXOCTRL_AUTOSTARTEM0EM1);
    EnableHFXO();
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
    EnableLFRCO();
    while (!LFRCOReady());
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
#else
    if (LFXOReady())
    {
#endif
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
        EnableLFRCO();
        while (!LFRCOReady());
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