/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/CMU.h
 *
 * Convenience wrapper around the EFM32 Series 1 Clock Management Unit
 */

#pragma once

#include <base/base.h>

#undef CMU
#define CMU	CM_PERIPHERAL(_CMU, CMU_BASE)

#if EFM32_LFXO_FREQUENCY && EFM32_LFXO_FREQUENCY != MONO_FREQUENCY
#error "Only 32768 Hz LF crystal is supported"
#endif

class _CMU : public CMU_TypeDef
{
public:
    //!< Initial clock configuration, should be called from startup code
    void Configure();

    bool HFRCOEnabled() { return STATUS & CMU_STATUS_HFRCOENS; }
    bool HFRCOReady() { return STATUS & CMU_STATUS_HFRCORDY; }
    bool HFRCOBusy() { return SYNCBUSY & CMU_SYNCBUSY_HFRCOBSY; }
    bool HFXOEnabled() { return STATUS & CMU_STATUS_HFXOENS; }
    bool HFXOReady() { return STATUS & CMU_STATUS_HFXORDY; }
    bool HFXOSelected() { return HFCLKSTATUS == CMU_HFCLKSTATUS_SELECTED_HFXO; }
    bool AUXHFRCOEnabled() { return STATUS & CMU_STATUS_AUXHFRCOENS; }
    bool AUXHFRCOReady() { return STATUS & CMU_STATUS_AUXHFRCORDY; }
    bool LFRCOEnabled() { return STATUS & CMU_STATUS_LFRCOENS; }
    bool LFRCOReady() { return STATUS & CMU_STATUS_LFRCORDY; }
    bool LFXOEnabled() { return STATUS & CMU_STATUS_LFXOENS; }
    bool LFXOReady() { return STATUS & CMU_STATUS_LFXORDY; }
#ifdef CMU_STATUS_USHFRCOENS
    bool USHFRCOEnabled() { return STATUS & CMU_STATUS_USHFRCOENS; }
    bool USHFRCOReady() { return STATUS & CMU_STATUS_USHFRCORDY; }
#endif

    void EnableHFRCO() { OSCENCMD = CMU_OSCENCMD_HFRCOEN; }
    void DisableHFRCO() { OSCENCMD = CMU_OSCENCMD_HFRCODIS; }
    void EnableHFXO() { OSCENCMD = CMU_OSCENCMD_HFXOEN; }
    void DisableHFXO() { OSCENCMD = CMU_OSCENCMD_HFXODIS; }
    void EnableAUXHFRCO() { OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN; }
    void DisableAUXHFRCO() { OSCENCMD = CMU_OSCENCMD_AUXHFRCODIS; }
    void EnableLFRCO() { OSCENCMD = CMU_OSCENCMD_LFRCOEN; }
    void DisableLFRCO() { OSCENCMD = CMU_OSCENCMD_LFRCODIS; }
    void EnableLFXO() { OSCENCMD = CMU_OSCENCMD_LFXOEN; }
    void DisableLFXO() { OSCENCMD = CMU_OSCENCMD_LFXODIS; }
#ifdef CMU_OSCENCMD_USHFRCOEN
    void EnableUSHFRCO() { OSCENCMD = CMU_OSCENCMD_USHFRCOEN; }
    void DisableUSHFRCO() { OSCENCMD = CMU_OSCENCMD_USHFRCODIS; }
#endif

#if EFM32_HFXO_FREQUENCY && EFM32_WAIT_FOR_HFXO
    //! Configures clocks before going into deep sleep
    void DeepSleepPrepare() {}
    //! Restores clocks after wakeup from deep sleep
    void DeepSleepRestore();
    //! Estimates microseconds needed to restore full program operation after wakeup from deep sleep
    unsigned DeepSleepRestoreMicroseconds();
#else
    //! Configures clocks before going into deep sleep
    void DeepSleepPrepare() {}
    //! Restores clocks after wakeup from deep sleep
    void DeepSleepRestore() {}
    //! Estimates microseconds needed to restore full program operation after wakeup from deep sleep
    unsigned DeepSleepRestoreMicroseconds() { return 16; } // datasheet specifications for various models are around 12 us
#endif

    void EnableLE()
    {
        EFM32_BITSET(HFBUSCLKEN0, CMU_HFBUSCLKEN0_LE);
        EFM32_BITSET(CTRL, CMU_CTRL_WSHFLE);
        EFM32_BITSET(HFPRESC, CMU_HFPRESC_HFCLKLEPRESC_DIV4);
    }

    bool RTCCEnabled() { return LFECLKEN0 & CMU_LFECLKEN0_RTCC; }
    void EnableRTCC() { EnableLE(); EFM32_BITSET(LFECLKEN0, CMU_LFECLKEN0_RTCC); }

#ifdef CMU_HFBUSCLKEN0_GPIO
    bool GPIOEnabled() { return HFBUSCLKEN0 & CMU_HFBUSCLKEN0_GPIO; }
    void EnableGPIO() { EFM32_BITSET(HFBUSCLKEN0, CMU_HFBUSCLKEN0_GPIO); }
#endif

#ifdef CMU_HFBUSCLKEN0_USB
    bool USBEnabled() { return HFBUSCLKEN0 & CMU_HFBUSCLKEN0_USB; }
    void EnableUSB()
    {
        EFM32_BITSET(HFBUSCLKEN0, CMU_HFBUSCLKEN0_USB);
#if EFM32_HFXO_FREQUENCY == 48000000
        USBCTRL = CMU_USBCTRL_USBCLKEN | CMU_USBCTRL_USBCLKSEL_HFXO;
#elif EFM32_HFXO_FREQUENCY == 24000000
        USBCTRL = CMU_USBCTRL_USBCLKEN | CMU_USBCTRL_USBCLKSEL_HFXOX2;
#else
        EnableUSHFRCO();
        USHFRCOCTRL = DEVINFO->USHFRCOCAL13;
        USBCTRL = CMU_USBCTRL_USBCLKEN | CMU_USBCTRL_USBCLKSEL_USHFRCO;
        USBCRCTRL |= CMU_USBCRCTRL_USBCREN;
#endif
    }
#endif

private:
    void IRQHandler();
};
