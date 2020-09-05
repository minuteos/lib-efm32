/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/base/platform_post.h
 */

#include_next <base/platform_post.h>

#ifdef _SILICON_LABS_32B_SERIES_1
#define EFM32_AUXHFRCO_FREQUENCY    16000000
#endif
