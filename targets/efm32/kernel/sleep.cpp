/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/kernel/sleep.cpp
 *
 * Compatibility with sleep.c from EMDRV
 */

#include <kernel/kernel.h>

BEGIN_EXTERN_C

void SLEEP_Init(void* sleepCallback, void* wakeupCallback)
{
    // sleep is managed by kernel, we do not support callbacks
    ASSERT(!sleepCallback);
    ASSERT(!wakeupCallback);
}

void SLEEP_SleepBlockBegin(uint32_t mode)
{
    if (mode == 2)
    {
        // only EM2 can be blocked, we never go to EM3
        PLATFORM_DEEP_SLEEP_DISABLE();
    }
}

void SLEEP_SleepBlockEnd(uint32_t mode)
{
    if (mode == 2)
    {
        // only EM2 can be blocked, we never go to EM3
        PLATFORM_DEEP_SLEEP_ENABLE();
    }
}

uint32_t SLEEP_LowestEnergyModeGet()
{
#if CORTEX_DEEP_SLEEP_ENABLED
    // EM2 is possible unless blocked
    if (PLATFORM_DEEP_SLEEP_ENABLED())
    {
        return 2;
    }
#endif

    // EM1 is always possible
    return 1;
}

END_EXTERN_C
