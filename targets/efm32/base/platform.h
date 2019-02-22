/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * targets/efm32/base/platform.h
 */

#pragma once

#define CORTEX_STARTUP_HARDWARE_INIT	_efm32_startup

#ifndef SWV_BAUD_RATE
#define SWV_BAUD_RATE   1000000
#endif

#include_next <base/platform.h>

extern void _efm32_startup();
