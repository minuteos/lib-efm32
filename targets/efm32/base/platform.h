/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * targets/efm32/base/platform.h
 */

#pragma once

#define CORTEX_STARTUP_HARDWARE_INIT	_efm32_startup
#define CORTEX_STARTUP_BEFORE_C_INIT	_efm32_c_startup

#ifndef SWV_BAUD_RATE
#define SWV_BAUD_RATE   3200000
#endif

#ifndef EFM32_WATCHDOG_TIMEOUT
#define EFM32_WATCHDOG_TIMEOUT  100
#endif

#if EFM32_WATCHDOG_TIMEOUT
#define PLATFORM_WATCHDOG_HIT           _efm32_hit_watchdog
#else
#define PLATFORM_WATCHDOG_HIT()
#endif

#include_next <base/platform.h>

extern void _efm32_startup();
extern void _efm32_c_startup();
#if EFM32_WATCHDOG_TIMEOUT
extern void _efm32_hit_watchdog();
#endif
extern void _efm32_irq_clearing_handler(void* pIFC);

#ifdef __cplusplus
ALWAYS_INLINE void EFM32_SetIRQClearingHandler(IRQn_Type IRQn, volatile uint32_t& ifc) { Cortex_SetIRQHandler(IRQn, GetDelegate(_efm32_irq_clearing_handler, (void*)&ifc)); }
#endif
