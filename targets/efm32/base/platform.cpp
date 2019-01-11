/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * targets/efm32/startup.cpp
 */

#include <base/base.h>

void _efm32_startup()
{
#if TRACE
    // enable clock to GPIO
#ifdef CMU_HFBUSCLKEN0_GPIO
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_GPIO;
#endif

    // enable default SWO pin (PF2)
    MODMASK(GPIO->P[5].MODEL, _GPIO_P_MODEL_MODE2_MASK, GPIO_P_MODEL_MODE2_PUSHPULL);
    GPIO->ROUTEPEN |= GPIO_ROUTEPEN_SWVPEN;

	// enable AUXHFRCO
	CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;
	while (!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));

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
}
