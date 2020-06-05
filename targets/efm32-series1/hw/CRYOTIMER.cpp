/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/CRYOTIMER.cpp
 */

#include <hw/CRYOTIMER.h>
#include <hw/CMU.h>

void CryoTimer::WakeFromEM4(unsigned milliseconds)
{
    CMU->EnableCRYOTIMER();

    // stop the timer if used for anything else
    CTRL = 0;
    IEN = 0;
    IFC = ~0u;

    if (milliseconds)
    {
        // period is selected only roughly, based on order
        PERIODSEL = 32 - __CLZ(milliseconds - 1);
        EM4WUEN = 1;
        // start the timer
        CTRL = CRYOTIMER_CTRL_EN | CRYOTIMER_CTRL_OSCSEL_ULFRCO;
    }
}
