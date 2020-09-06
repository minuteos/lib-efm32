/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series2/hw/PDM.h
 */

#pragma once

#include <base/base.h>

#undef PDM
#define PDM CM_PERIPHERAL(_PDM, PDM_BASE)

class _PDM : public PDM_TypeDef
{
public:
    constexpr unsigned Index() { return 0; }

    void Enable() { CMU->EnablePDM(); EN = PDM_EN_EN; }
    void Disable() { EN = 0; CMU->DisablePDM(); }

    //! Configures the CLK pin
    void ConfigureClk(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, GPIO_ROUTE_ARGS(PDM, CLK)); }
    //! Configures the DAT0 pin
    void ConfigureDat0(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, GPIO_ROUTE_ARGS_NOPEN(PDM, DAT0)); }
    //! Configures the DAT1 pin
    void ConfigureDat1(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, GPIO_ROUTE_ARGS_NOPEN(PDM, DAT1)); }
};
