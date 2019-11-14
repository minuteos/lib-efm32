/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/PRS.cpp
 */

#include <hw/PRS.h>
#include <hw/CMU.h>

void _PRS::Configure1MHzDMA(unsigned channel)
{
    // HFEXPCLK = 1 MHz clock
    CMU->PrescaleHFEXPCLK(32);
    CMU->ConfigureClockOutput0(_CMU::OutputClock::HFEXPCLK);
    EnableClock();

    Channel(channel).Setup(PRSChannel::EdgeRising | PRSChannel::SourceCMU_Clock0);
    Channel(channel).DMARequest(PRS_1MHZ_DMA);
}

PRSChannelHandle _PRS::GetChannel(PRSChannel::Flags flags)
{
    EnableClock();

    for (unsigned i = 0; i < countof(CH); i++)
    {
        if (CH[i].CTRL == flags)
        {
            return i;
        }
    }

    for (unsigned i = 0; i < countof(CH); i++)
    {
        if (CH[i].CTRL == 0)
        {
            CH[i].CTRL = flags;
            return i;
        }
    }

    ASSERT(false);
    return ~0u;
}
