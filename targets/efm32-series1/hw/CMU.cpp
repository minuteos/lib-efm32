/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/CMU.cpp
 */

#include <hw/CMU.h>

#include <em_cmu.h>

void _CMU::Configure()
{
#if !EFM32_NO_LFXO
    EnableLFXO();
#endif
#if defined(EFM32_HFXO_FREQUENCY)
    EnableHFXO();
#endif

#if (!EFM32_NO_LFXO && !EFM32_WAIT_FOR_LFXO) || \
    (defined(EFM32_HFXO_FREQUENCY) && !EFM32_WAIT_FOR_HFXO)
    // configure CMU interrupt for when the LFXO starts
    Cortex_SetIRQHandler(CMU_IRQn, GetDelegate(this, &_CMU::IRQHandler));
    NVIC_EnableIRQ(CMU_IRQn);
#endif

    // first configure the low frequency oscillator
#if EFM32_NO_LFXO
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
        DBGCL("CMU", "Waiting for LFXO...");
        while (!LFXOReady());
#else
    if (LFXOReady())
    {
#endif
        // LFXO is running
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
        // do not set LFECLKSEL yet, will be set once LFXO is stable
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
    EFM32_BITSET(IEN, CMU_IEN_LFXORDY);
#endif
#endif
}

void _CMU::IRQHandler()
{
    UNUSED auto mask = IFC;

#if !EFM32_NO_LFXO && !EFM32_WAIT_FOR_LFXO
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

#if defined(EFM32_HFXO_FREQUENCY) && !EFM32_WAIT_FOR_HFXO
    if (mask & CMU_IFC_HFXORDY)
    {
        HFCLKSEL = CMU_HFCLKSEL_HF_HFXO;
        DisableHFRCO();
    }
#endif
}