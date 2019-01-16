/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/CMU.cpp
 */

#include <hw/CMU.h>

void _CMU::EnableLFO()
{
#if EFM32_NO_LFXO
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
    EnableLFXO();
    if (!LFXOReady())
    {
        DBGCL("CMU", "Waiting for LFXO...");
        while (!LFXOReady());
    }
#else
    if (LFXOReady() && LFXOEnabled())
#endif
    {
        // LFXO still running
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
        // configure CMU interrupt for when the LFXO starts
        Cortex_SetIRQHandler(CMU_IRQn, GetDelegate(this, &_CMU::IRQHandler));
        NVIC_EnableIRQ(CMU_IRQn);
        EFM32_BITSET(IEN, CMU_IEN_LFXORDY);

        EnableLFXO();
        EnableLFRCO();
        while (!LFRCOReady());
        LFACLKSEL = CMU_LFACLKSEL_LFA_LFRCO;
        LFBCLKSEL = CMU_LFBCLKSEL_LFB_LFRCO;
#ifdef CMU_LFCCLKSEL_LFC_LFRCO
        LFCCLKSEL = CMU_LFCCLKSEL_LFC_LFRCO;
#endif
        LFECLKSEL = CMU_LFECLKSEL_LFE_LFRCO;
        // do not set LFECLKSEL yet, will be set once LFXO is stable
    }
#endif
#endif
}

void _CMU::IRQHandler()
{
    UNUSED auto mask = IFC;

#if !defined(EFM32_NO_LFXO) && !defined(EFM32_WAIT_FOR_LFXO)
    if (mask & CMU_IFC_LFXORDY)
    {
        DBGCL("CMU", "Switching to LFXO");
        EFM32_BITCLR(IEN, CMU_IFC_LFXORDY);
        LFACLKSEL = CMU_LFACLKSEL_LFA_LFXO;
        LFBCLKSEL = CMU_LFBCLKSEL_LFB_LFXO;
#ifdef CMU_LFCCLKSEL_LFC_LFXO
        LFCCLKSEL = CMU_LFCCLKSEL_LFC_LFXO;
#endif
        LFECLKSEL = CMU_LFECLKSEL_LFE_LFXO;
    }
#endif
}