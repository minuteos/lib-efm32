/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-rtcc/kernel/platform_post.h
 */

#pragma once

// need RTCC for CORTEX_SCHEDULE/CLEAN_WAKEUP callbacks defined in base/platform.h
#include <hw/RTCC.h>
