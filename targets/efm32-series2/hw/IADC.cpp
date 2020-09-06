/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series2/hw/IADC.cpp
 */

#include <hw/IADC.h>

async(IADC::_MeasureSingle, uint32_t singleCtrl)
async_def_return(false);
