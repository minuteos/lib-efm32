/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/EMU.cpp
 */

#include <hw/EMU.h>

#define MYDBG(...)  DBGCL("EMU", __VA_ARGS__)

#ifdef Ckernel

async(_EMU::SetR5VOutputLevel, int mv100)
async_def()
{
    MYDBG("Current R5V output: %.1qV", R5VOutputLevel());
    R5VOutputLevel(mv100);
    await_mask(IF, EMU_IF_R5VVSINT, EMU_IF_R5VVSINT);
    MYDBG("New R5V output: %.1qV", R5VOutputLevel());
}
async_end

#endif
