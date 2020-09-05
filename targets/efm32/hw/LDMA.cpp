/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/LDMA.cpp
 */

#include <hw/LDMA.h>

LDMAChannelHandle LDMAController::GetChannel(uint32_t srcDef, bool reuse)
{
    EnableClock();

    if (reuse)
    {
        for (unsigned i = 0; i < countof(CH); i++)
        {
            if (REQSEL(i) == srcDef)
            {
                return i;
            }
        }
    }

    for (unsigned i = 0; i < countof(CH); i++)
    {
        if (REQSEL(i) == 0)
        {
            REQSEL(i) = srcDef;
            return i;
        }
    }

    ASSERT(0);
    return ~0u;
}

unsigned LDMAController::FreeChannels()
{
    EnableClock();

    unsigned cnt = 0;

    for (unsigned i = 0; i < countof(CH); i++)
    {
        if (REQSEL(i) == 0)
        {
            cnt++;
        }
    }

    return cnt;
}

/*!
 * Waits for all the bits in the mask to set their done flag
 *
 * The order in which the bits become set is not important,
 * nor do they have to be set at once, but the wait ends only
 * when all bits have been set at least once
 */
async(LDMAController::WaitForDoneMask, uint32_t mask)
async_def()
{
    // make sure the IRQ is enabled and configured
    Cortex_SetIRQWakeup(LDMA_IRQn);
    NVIC_ClearPendingIRQ(LDMA_IRQn);
    NVIC_EnableIRQ(LDMA_IRQn);

    // do not clear the interrupts before waiting,
    // as some bits may be already set

    EFM32_BITSET_REG(IEN, mask);
    do
    {
        // wait for any of the remaining bits to become set
        await_mask_not(IF, IEN & mask, 0);
        EFM32_BITCLR_REG(IEN, IF & mask);
        NVIC_ClearPendingIRQ(LDMA_IRQn);
    } while (IEN & mask);

    EFM32_IFC(this) = mask;
}
async_end
