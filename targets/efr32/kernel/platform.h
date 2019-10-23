/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efr32/kernel/platform.h
 */

#if EFR32_RAIL_SLEEP
bool _RailTimerSync_Suspend(mono_t& wakeDelayTicks);
void _RailTimerSync_Resume();

#define CORTEX_DEEP_SLEEP_BEFORE()  _RailTimerSync_Suspend(wakeDelayTicks)
#define CORTEX_DEEP_SLEEP_AFTER()   _RailTimerSync_Resume()
#endif

#include_next <kernel/platform.h>

#ifndef EFR32_RAIL_SLEEP_EXTRA_TICKS
#define EFR32_RAIL_SLEEP_EXTRA_TICKS    1
#endif
