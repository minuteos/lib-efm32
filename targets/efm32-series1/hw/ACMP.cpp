/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/ACMP.cpp
 */

#include <hw/ACMP.h>

res_pair_t ACMP::CalculateHysteresis(uint32_t mv)
{
    uint32_t baseRef = 0;
    switch (INPUTSEL & (_ACMP_INPUTSEL_NEGSEL_MASK | _ACMP_INPUTSEL_VASEL_MASK | _ACMP_INPUTSEL_VBSEL_MASK | _ACMP_INPUTSEL_VLPSEL_MASK))
    {
    case ACMP_INPUTSEL_NEGSEL_VADIV | ACMP_INPUTSEL_VASEL_VDD:
    case ACMP_INPUTSEL_NEGSEL_VLP | ACMP_INPUTSEL_VLPSEL_VADIV | ACMP_INPUTSEL_VASEL_VDD:
        baseRef = 3300;	// TODO: VDD may be different
        break;
    case ACMP_INPUTSEL_NEGSEL_VBDIV | ACMP_INPUTSEL_VBSEL_1V25:
    case ACMP_INPUTSEL_NEGSEL_VLP | ACMP_INPUTSEL_VLPSEL_VBDIV | ACMP_INPUTSEL_VBSEL_1V25:
        baseRef = 1250;
        break;
    case ACMP_INPUTSEL_NEGSEL_VBDIV | ACMP_INPUTSEL_VBSEL_2V5:
    case ACMP_INPUTSEL_NEGSEL_VLP | ACMP_INPUTSEL_VLPSEL_VBDIV | ACMP_INPUTSEL_VBSEL_2V5:
        baseRef = 2500;
        break;
    default:
        ASSERT(false);
        return res_pair_t();
    }

    // find the lowest divisor which could possibly return a good match
    uint32_t div = mv * 64 / baseRef;
    if (div > 64) div = 64;
    else if (div < 1) div = 1;

    uint32_t ref = baseRef * div / 64;

#ifdef EFM32_ACMP_USE_FINE_HYSTERESIS
    // hysteresis finetuning, seems to be triggering way off in practice
    static constexpr byte hyst[] = { 0, 18, 32, 44, 55, 65, 77, 86 };
    while (div > 1 && ref + 45 > mv)
    {
        div--;
        ref = baseRef * div / 64;
    }

    uint bestDiff = ~0u, bestMatch = 0, bestHys = 0, bestDiv = 0;
    do
    {
        bool neg = ref > mv;
        for (uint i = 0; i < countof(hyst); i++)
        {
            uint match = neg ? ref - hyst[i] : ref + hyst[i];
            if ((int)match < 0)
                continue;	// must not go below zero
            uint hys = neg ? i + 8 : i;
            uint diff = abs((int)(match - mv));
            if (diff < bestDiff)
            {
                bestMatch = match;
                bestHys = hys;
                bestDiv = div;
                if (!(bestDiff = diff))
                    goto found;
            }
        }

        div++;
        ref = baseRef * div / 64;
    } while (div <= 64 && ref - 45 <= mv);
found:
    DBGC("ACMP", "requested level %d mV, best match %d * %d / 64 = %d %c %d = %d\n", mv, baseRef, bestDiv, baseRef * bestDiv / 64, bestHys & 8 ? '-' : '+', hyst[bestHys & 7],
        baseRef * bestDiv / 64 + (hyst[bestHys & 7] * (bestHys & 8 ? -1 : 1)));
    bestDiv--;
    return PACK64(bestMatch, bestHys | bestDiv << 16 | bestDiv << 24);
#else
    if (div < 64)
    {
        // try the next highest
        uint32_t ref2 = baseRef * (div + 1) / 64;
        if (abs(int(ref2 - mv)) < abs(int(ref - mv)))
        {
            div++;
            ref = ref2;
        }
    }

    DBGC("ACMP", "requested level %d mV, best match %d * %d / 64 = %d\n", mv, baseRef, div, ref);
    div--;
    return RES_PAIR(ref, div << 16 | div << 24);
#endif
}
