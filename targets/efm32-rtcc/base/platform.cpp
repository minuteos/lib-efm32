/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-rtcc/platform.cpp
 *
 * Provides platform-specific implementations for EFM32 MCUs equipped
 * with RTCC module
 */

#include <base/base.h>

OPTIMIZE uint32_t _efm32_mono_us()
{
    if (!CMU->RTCCEnabled())
        return 0;

    auto hi = RTCC->CNT;
    auto lo = RTCC->COMBCNT;

    // if top 17 bits do not equal bottom 17 bits of CNT, COMBCNT has wrapped
    // precisely between the two reads
    if ((hi & MASK(17)) != lo >> 15)
        hi++;

    auto mono64 = ((int64_t)hi << 15) | lo;   // no need to mask lo as the bits must be equal
    return (mono64 * ((int64_t)1000000 << 17)) >> 32;
}
