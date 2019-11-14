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

#if EFM32_HFXO_FREQUENCY
    static constexpr unsigned GetCoreFrequency() { return EFM32_HFXO_FREQUENCY; }
#elif EFM32_HFRCO_FREQUENCY
    static constexpr unsigned GetCoreFrequency() { return EFM32_HFRCO_FREQUENCY; }
#else
    static constexpr unsigned GetCoreFrequency() { return 19000000; }
#endif

    void PrescaleHFEXPCLK(unsigned prescaler) { HFEXPPRESC = (prescaler - 1) << 8; }

    enum struct OutputClock
    {
        None = _CMU_CTRL_CLKOUTSEL0_DISABLED,
        ULFRCO = _CMU_CTRL_CLKOUTSEL0_ULFRCO,
        LFRCO = _CMU_CTRL_CLKOUTSEL0_LFRCO,
        LFXO = _CMU_CTRL_CLKOUTSEL0_LFXO,
        HFXO = _CMU_CTRL_CLKOUTSEL0_HFXO,
        HFEXPCLK = _CMU_CTRL_CLKOUTSEL0_HFEXPCLK,
        ULFRCOQ = _CMU_CTRL_CLKOUTSEL0_ULFRCOQ,
        LFRCOQ = _CMU_CTRL_CLKOUTSEL0_LFRCOQ,
        LFXOQ = _CMU_CTRL_CLKOUTSEL0_LFXOQ,
        HFXOQ = _CMU_CTRL_CLKOUTSEL0_HFXOQ,
        HFSRCCLK = _CMU_CTRL_CLKOUTSEL0_HFSRCCLK,
    };

    void ConfigureClockOutput0(OutputClock clk) { MODMASK(CTRL, _CMU_CTRL_CLKOUTSEL0_MASK, uint32_t(clk) << _CMU_CTRL_CLKOUTSEL0_SHIFT); }
    void ConfigureClockOutput1(OutputClock clk) { MODMASK(CTRL, _CMU_CTRL_CLKOUTSEL1_MASK, uint32_t(clk) << _CMU_CTRL_CLKOUTSEL1_SHIFT); }

    void EnableLE()
    {
        EFM32_BITSET(HFBUSCLKEN0, CMU_HFBUSCLKEN0_LE);
        EFM32_BITSET(CTRL, CMU_CTRL_WSHFLE);
        EFM32_BITSET(HFPRESC, CMU_HFPRESC_HFCLKLEPRESC_DIV4);
    }

    bool RTCCEnabled() { return (HFBUSCLKEN0 & CMU_HFBUSCLKEN0_LE) && (LFECLKEN0 & CMU_LFECLKEN0_RTCC); }
    void EnableRTCC() { EnableLE(); EFM32_BITSET(LFECLKEN0, CMU_LFECLKEN0_RTCC); }

#ifdef CMU_HFBUSCLKEN0_GPIO
    bool GPIOEnabled() { return HFBUSCLKEN0 & CMU_HFBUSCLKEN0_GPIO; }
    void EnableGPIO() { EFM32_BITSET(HFBUSCLKEN0, CMU_HFBUSCLKEN0_GPIO); }
#endif

#ifdef CMU_HFPERCLKEN0_I2C0
    bool I2CEnabled(unsigned index) { ASSERT(index < I2C_COUNT); return HFBUSCLKEN0 & (CMU_HFPERCLKEN0_I2C0 << index); }
    void EnableI2C(unsigned index) { ASSERT(index < I2C_COUNT); EFM32_BITSET(HFPERCLKEN0, CMU_HFPERCLKEN0_I2C0 << index); }
#endif

#ifdef CMU_HFPERCLKEN0_ADC0
    bool ADCEnabled(unsigned index) { ASSERT(index < ADC_COUNT); return HFPERCLKEN0 & (CMU_HFPERCLKEN0_ADC0 << index); }
    void EnableADC(unsigned index) { ASSERT(index < ADC_COUNT); EFM32_BITSET(HFPERCLKEN0, CMU_HFPERCLKEN0_ADC0 << index); }
    void DisableADC(unsigned index) { ASSERT(index < ADC_COUNT); EFM32_BITCLR(HFPERCLKEN0, CMU_HFPERCLKEN0_ADC0 << index); }
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

#ifdef CMU_HFBUSCLKEN0_PRS
    bool PRSEnabled() { return HFBUSCLKEN0 & CMU_HFBUSCLKEN0_PRS; }
    void EnablePRS() { EFM32_BITSET(HFBUSCLKEN0, CMU_HFBUSCLKEN0_PRS); }
#endif

#ifdef CMU_HFBUSCLKEN0_LDMA
    bool LDMAEnabled() { return HFBUSCLKEN0 & CMU_HFBUSCLKEN0_LDMA; }
    void EnableLDMA() { EFM32_BITSET(HFBUSCLKEN0, CMU_HFBUSCLKEN0_LDMA); }
#endif

private:
    void IRQHandler();
};
