/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efr32/kernel/RailTimerSync.cpp
 *
 * Support for the RAIL protocol timer, calls RAIL_Sleep and RAIL_Wake
 * before/after deep sleep
*/

#include <kernel/kernel.h>

#include <rail.h>

static struct
{
    bool active;
    bool enabled;
} l_state = { 0 };

bool _RailTimerSync_Suspend(mono_t& wakeDelayTicks)
{
    if (!l_state.enabled)
    {
        // no special handling
        l_state.active = false;
        return true;
    }

    wakeDelayTicks += EFR32_RAIL_SLEEP_EXTRA_TICKS;
    if (RAIL_Sleep(MonoToMicroseconds(wakeDelayTicks), &l_state.active))
    {
        l_state.active = false;
    }

    return l_state.active;
}

void _RailTimerSync_Resume()
{
    if (l_state.active)
    {
        l_state.active = false;
        RAIL_Wake(0);
    }
}

//! extract RAIL handle from the Bluetooth stack
extern "C" RAIL_Handle_t BTLE_LL_GetRadioHandle();

//! overrides standard Bluetooth stack sleep_init so we know when RAIL_ConfigSleep has been called
extern "C" void sleep_init()
{
    auto handle = BTLE_LL_GetRadioHandle();
    RAIL_ConfigSleep(handle, RAIL_SLEEP_CONFIG_TIMERSYNC_ENABLED);
    l_state.enabled = true;
}

//! never called, allows the linker to strip a lot of sleep code from libbluetooth
extern "C" void BG_Sleep()
{
    ASSERT(false);
}

//! never called, allows the linker to strip a lot of sleep code from libbluetooth
extern "C" void SleepAndSyncProtimer()
{
    ASSERT(false);
}
