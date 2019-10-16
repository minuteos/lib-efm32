/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * targets/efm32/startup.cpp
 */

#include <base/base.h>

#include <em_chip.h>

#include <hw/CMU.h>
#include <hw/EMU.h>
#include <hw/GPIO.h>

void _efm32_startup()
{
    // apply EMLIB errata fixes
    // this is an inline function, so we actually don't need the full emlib component for it, just the headers
    CHIP_Init();

    // use the "clear interrupts on read" semantics (reading IFC is the same as IFC = IF)
    // also enable bus faults when accessing disabled peripherals
    // silently ignoring these errors sometimes causes very hard to find bugs
    if (MSC->LOCK & MSC_LOCK_LOCKKEY_LOCKED)
        MSC->LOCK = MSC_LOCK_LOCKKEY_UNLOCK;
    MSC->CTRL |= MSC_CTRL_IFCREADCLEAR | MSC_CTRL_CLKDISFAULTEN;
    MSC->LOCK = MSC_LOCK_LOCKKEY_LOCK;

#if TRACE
    // enable clock to GPIO
    CMU->EnableGPIO();

    // enable default SWO pin (PF2)
    MODMASK(GPIO->P[5].MODEL, _GPIO_P_MODEL_MODE2_MASK, GPIO_P_MODEL_MODE2_PUSHPULL);
    GPIO->ROUTEPEN |= GPIO_ROUTEPEN_SWVPEN;
#if EFM32_GPIO_DRIVE_CONTROL
    GPIO->P[5].CTRL = EFM32_GPIO_DRIVE_DEFAULT;
#endif

    // enable AUXHFRCO
    CMU->EnableAUXHFRCO();
    while (!CMU->AUXHFRCOReady());

    ITM->LAR = 0xC5ACCE55;   // unlock

    // disable ITM
    ITM->TER = 0;
    ITM->TCR = 0;

    // initialize TPIU
    TPI->SPPR = 2;		// NRZ protocol
    TPI->ACPR = (19000000 / SWV_BAUD_RATE) - 1;
    TPI->FFCR = 0x100;  // EnFTC

    // enable ITM
    ITM->TCR = 0x10009;
    ITM->TER = ~0u;		// enable all channels
#endif

    EMU->Configure();
    CMU->Configure();

#ifdef EFM32_RTC
    // start RTC as soon as possible
    EFM32_RTC->Configure();
#endif
}
