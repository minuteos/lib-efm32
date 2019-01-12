/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-rtcc/kernel/platform.h
 * 
 * Implements monotonic clock required by the kernel using EFM32's RTCC timer
 */

#pragma once

#define MONO_CLOCKS	(RTCC->COMBCNT)
#define MONO_FREQUENCY 32768
#define MONO_US     _efm32_mono_us()    // used for DBG timestamps

extern uint32_t _efm32_mono_us();

typedef uint32_t mono_t;

#define CORTEX_SCHEDULE_WAKEUP  RTCC->SetupWake
#define CORTEX_CLEAN_WAKEUP  RTCC->DisableWake

#define EFM32_RTC   RTCC

#include_next <base/platform.h>

#ifdef __cplusplus

#include <hw/RTCC.h>

#endif
