/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * platform.h
 */


#pragma once

#include_next <base/platform.h>

#if !BOOTLOADER

#ifndef CORTEX_DEEP_SLEEP_ENABLED
#define CORTEX_DEEP_SLEEP_ENABLED   1   // enabled by default
#endif

#endif

#define CORTEX_DEEP_SLEEP_PREPARE       CMU->DeepSleepPrepare
#define CORTEX_DEEP_SLEEP_RESTORE       CMU->DeepSleepRestore
#define CORTEX_DEEP_SLEEP_RESTORE_US    CMU->DeepSleepRestoreMicroseconds
