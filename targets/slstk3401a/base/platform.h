/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * targets/slstk3401a/base/platform.h
 * 
 * Overrides for the EFM32 Pearl Gecko starter kit
 */

#pragma once

// the onboard debugger runs at 4.5 MHz and cannot hit 1MHz debug clock 
#define SWV_BAUD_RATE   900000

#include_next <base/platform.h>
