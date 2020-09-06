/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series2/hw/CMU.h
 */

#pragma once

#include <base/base.h>

#undef CMU
#define CMU CM_PERIPHERAL(_CMU, CMU_BASE)

#ifndef EFM32_LFXO_LOAD_CAPACITANCE
#define EFM32_LFXO_LOAD_CAPACITANCE 12.5
#endif

#ifndef EFM32_LFXO_GAIN
#define EFM32_LFXO_GAIN             2
#endif

class _CMU : public CMU_TypeDef
{
public:
#define _CMU_PERIPHERAL(reg, name, flag) \
    bool name ## Enabled() const { return reg & (flag); } \
    void Enable ## name() { reg ## _SET = flag; } \
    void Disable ## name() { reg ## _CLR = flag; }

#define CMU_PERIPHERAL(reg, name) _CMU_PERIPHERAL(reg, name, CMU_ ## reg ## _ ## name)

#define CMU_INDEXED_PERIPHERAL(reg, name) \
    bool name ## Enabled(unsigned index) const { ASSERT(index < name ## _COUNT); return reg & (CMU_ ## reg ## _ ## name ## 0 << index); } \
    void Enable ## name(unsigned index) { ASSERT(index < name ## _COUNT); reg ## _SET = CMU_ ## reg ## _ ## name ## 0 << index; } \
    void Disable ## name(unsigned index) { ASSERT(index < name ## _COUNT); reg ## _CLR = CMU_ ## reg ## _ ## name ## 0 << index; }

    _CMU_PERIPHERAL(CLKEN0, LDMA, CMU_CLKEN0_LDMA | CMU_CLKEN0_LDMAXBAR)
#pragma push_macro("GPCRC")
#undef GPCRC
    CMU_PERIPHERAL(CLKEN0, GPCRC)
#pragma pop_macro("GPCRC")

#pragma push_macro("PDM")
#undef PDM
    CMU_PERIPHERAL(CLKEN0, PDM)
#pragma pop_macro("PDM")

#pragma push_macro("GPIO")
#undef GPIO
    CMU_PERIPHERAL(CLKEN0, GPIO)
#pragma pop_macro("GPIO")

    CMU_INDEXED_PERIPHERAL(CLKEN0, TIMER)
    CMU_INDEXED_PERIPHERAL(CLKEN0, USART)
    CMU_INDEXED_PERIPHERAL(CLKEN0, IADC)
    CMU_INDEXED_PERIPHERAL(CLKEN0, AMUXCP)
    CMU_INDEXED_PERIPHERAL(CLKEN0, LETIMER)
    CMU_INDEXED_PERIPHERAL(CLKEN0, WDOG)
    CMU_INDEXED_PERIPHERAL(CLKEN0, I2C)
    _CMU_PERIPHERAL(CLKEN0, RTCC, CMU_CLKEN0_RTCC | CMU_CLKEN0_BURAM)

#undef CMU_PERIPHERAL
#undef CMU_INDEXED_PERIPHERAL

#if EFM32_HFXO_FREQUENCY
    static constexpr unsigned GetCoreFrequency() { return EFM32_HFXO_FREQUENCY; }
#elif EFM32_HFRCO_FREQUENCY
    static constexpr unsigned GetCoreFrequency() { return EFM32_HFRCO_FREQUENCY; }
#else
    static constexpr unsigned GetCoreFrequency() { return 20000000; }
#endif
    static constexpr unsigned GetTraceFrequency() { return GetCoreFrequency(); }

#if EFM32_HFXO_FREQUENCY && !EFM32_WAIT_FOR_HFXO
    //! Configures clocks before going into deep sleep
    void DeepSleepPrepare()
    {
        // run from HFRCO after wakeup
        SYSCLKCTRL = _CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL;
        EM01GRPACLKCTRL = CMU_EM01GRPACLKCTRL_CLKSEL_HFRCODPLL;
        EM01GRPBCLKCTRL = CMU_EM01GRPBCLKCTRL_CLKSEL_HFRCODPLL;
    }
    //! Restores clocks after wakeup from deep sleep
    void DeepSleepRestore()
    {
        // go back to forced HFXO mode, SYSCLK will get switched when ready
        HFXO0->CTRL = HFXO_CTRL_FORCEEN;
    }
#else
    //! Configures clocks before going into deep sleep
    void DeepSleepPrepare() {}
    //! Restores clocks after wakeup from deep sleep
    void DeepSleepRestore() {}
#endif

#if EFM32_HFXO_FREQUENCY && EFM32_WAIT_FOR_HFXO
    //! Estimates microseconds needed to restore full program operation after wakeup from deep sleep
    unsigned DeepSleepRestoreMicroseconds() { return 1000; }
#else
    //! Estimates microseconds needed to restore full program operation after wakeup from deep sleep
    unsigned DeepSleepRestoreMicroseconds() { return 10; }
#endif

private:
    void LFXOReadyHandler();
    void HFXOReadyHandler();

    void EarlyConfigure()
    {
        // start on HFRCO running at 38 MHz
        CLKEN0_SET = CMU_CLKEN0_HFRCO0;
        // enable MSC, always used
        CLKEN1_SET = CMU_CLKEN1_MSC;
        HFRCO0->CAL = DEVINFO->HFRCODPLLCAL[12].HFRCODPLLCAL;
        SYSCLKCTRL = CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL;
    }

    void Configure();

    friend void _efm32_startup();
    friend void _efm32_c_startup();
};
