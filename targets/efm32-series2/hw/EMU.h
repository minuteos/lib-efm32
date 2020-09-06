/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series2/hw/EMU.h
 */

#pragma once

#include <base/base.h>

#undef EMU
#define EMU CM_PERIPHERAL(_EMU, EMU_BASE)

class _EMU : public EMU_TypeDef
{
private:
    void Configure();

    friend void _efm32_c_startup();
};
