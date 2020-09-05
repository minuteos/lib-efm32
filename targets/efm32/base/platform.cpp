/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * targets/efm32/startup.cpp
 */

#include <base/base.h>

#include <em_chip.h>

#include <hw/WDOG.h>
#include <hw/CMU.h>
#include <hw/EMU.h>
#include <hw/RMU.h>
#include <hw/MSC.h>
#include <hw/GPIO.h>

void _efm32_startup()
{
    // apply EMLIB errata fixes
    // this is an inline function, so we actually don't need the full emlib component for it, just the headers
    CHIP_Init();

    CMU->EarlyConfigure();

#if TRACE || MINTRACE
    // enable clock to GPIO
    CMU->EnableGPIO();

    GPIO->EnableTrace();

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    ITM->LAR = 0xC5ACCE55;   // unlock

    // disable ITM
    ITM->TER = 0;
    ITM->TCR = 0;

    // initialize TPIU
    TPI->SPPR = 2;		// NRZ protocol
    TPI->ACPR = ((CMU->GetTraceFrequency() + SWV_BAUD_RATE / 2) / SWV_BAUD_RATE) - 1;
    TPI->FFCR = 0x100;  // EnFTC

    // enable ITM
    ITM->TCR = (1 << ITM_TCR_TraceBusID_Pos) | ITM_TCR_SWOENA_Msk | ITM_TCR_ITMENA_Msk;
    ITM->TER = ~0u;		// enable all channels
#endif
}

void _efm32_c_startup()
{
#if EFM32_WATCHDOG_TIMEOUT
    // configure watchdog for a long timeout (for clocks init)
    WDOG0->Configure(2000);
#endif

    MSC->Configure();
    EMU->Configure();
    CMU->Configure();
    RMU->Configure();

#if EFM32_WATCHDOG_TIMEOUT
    PLATFORM_WATCHDOG_HIT();
    WDOG0->Sync();
    // lock watchdog configuration with configured timeout
    WDOG0->Configure(EFM32_WATCHDOG_TIMEOUT, true);
#endif

#ifdef EFM32_RTC
    // start RTC as soon as possible
    EFM32_RTC->Configure();
#endif
}

#if EFM32_WATCHDOG_TIMEOUT
void _efm32_hit_watchdog()
{
    WDOG0->Hit();
}
#endif

void _efm32_irq_clearing_handler(void* p)
{
    volatile uint32_t* pIFC = (volatile uint32_t*)p;
    *pIFC = *pIFC;
}

#if GECKO_SIGNATURE
__attribute__((section(".sig_gecko"))) uint8_t _efm32_signature[64];
#endif
