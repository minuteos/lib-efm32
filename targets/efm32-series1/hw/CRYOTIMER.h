/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/CRYOTIMER.h
 */

#pragma once

#include <base/base.h>

#undef CRYOTIMER
#define CRYOTIMER	CM_PERIPHERAL(CryoTimer, CRYOTIMER_BASE)

class CryoTimer : public CRYOTIMER_TypeDef
{
public:
    void WakeFromEM4(unsigned milliseconds);
};
