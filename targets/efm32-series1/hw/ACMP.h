/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/ACMP.h
 */

#pragma once

#include <base/base.h>

#include <hw/CMU.h>
#include <hw/GPIO.h>

#include <algorithm>

#undef ACMP0
#undef ACMP1
#define ACMP0   CM_PERIPHERAL(ACMP, ACMP0_BASE)
#define ACMP1   CM_PERIPHERAL(ACMP, ACMP1_BASE)

class ACMP : public ACMP_TypeDef
{
public:
    enum Flags
    {
        FlagEnabled = BIT(0),

        InactiveLow = 0,
        InactiveHigh = BIT(1),

        PinInvert = BIT(2),

        MasterDisableX = BIT(8),
        MasterDisableY = BIT(9),
        MasterDisableV = BIT(10),

        PowerSupplyAVDD = 0,
        PowerSupplyVREGVDD = 1 << 12,
        PowerSupplyIOVDD0 = 2 << 12,
        PowerSupplyIOVDD1 = 4 << 12,

        AccuracyLow = 0,
        AccuracyHigh = BIT(15),

        InputRangeFull = 0,
        InputRangeTopHalf = 1 << 18,
        InputRangeBottomHalf = 2 << 18,

        InterruptRising = BIT(20),
        InterruptFalling = BIT(21),

        BiasProgMin = 0 << 24,
        BiasProgDefault = 7 << 24,
        BiasProgMax = 31 << 24,

        FullBias = BIT(31),
    };

    enum Hysteresis
    {
        Hysteresis0,
        Hysteresis14mV,
        Hysteresis25mV,
        Hysteresis30mV,
        Hysteresis35mV,
        Hysteresis39mV,
        Hysteresis42mV,
        Hysteresis45mV,
        HysteresisMinus0,
        HysteresisMinus14mV,
        HysteresisMinus25mV,
        HysteresisMinus30mV,
        HysteresisMinus35mV,
        HysteresisMinus39mV,
        HysteresisMinus42mV,
        HysteresisMinus45mV,
    };

    enum ReferenceA
    {
        ReferenceVDD = 0,
    };

    enum ReferenceB
    {
        Reference1V25 = 0,
        Reference2V5 = BIT(22),
    };

    enum ReferenceLP
    {
        ReferenceLPVDD = 0,
        ReferenceLP1V25 = BIT(24),
        ReferenceLP2V5 = BIT(24) | BIT(22),
    };

    uint32_t Index() const { return ((uint32_t)this >> 10) & 15; }

    void EnableClock() { CMU->EnableACMP(Index()); }
    void Setup(Flags flags) { CTRL = flags; }
    void SetupInput(APORTX pos, APORTY neg) { INPUTSEL = pos | (neg << 8); }
    void SetupInput(APORTX pos, ReferenceA ref) { INPUTSEL = pos | (APORTY::VADIV << 8) | ref; }
    void SetupInput(APORTX pos, ReferenceB ref) { INPUTSEL = pos | (APORTY::VBDIV << 8) | ref; }
    void SetupInput(APORTX pos, ReferenceLP ref) { INPUTSEL = pos | (APORTY::VLP << 8) | ref; }

    ALWAYS_INLINE uint32_t SetupRising(uint32_t mv) { auto calc = CalculateHysteresis(mv); HYSTERESIS0 = RES_PAIR_SECOND(calc); return RES_PAIR_FIRST(calc); }
    ALWAYS_INLINE uint32_t SetupFalling(uint32_t mv) { auto calc = CalculateHysteresis(mv); HYSTERESIS1 = RES_PAIR_SECOND(calc); return RES_PAIR_FIRST(calc); }
    void SetupLevel(float level, Hysteresis rising = Hysteresis0, Hysteresis falling = Hysteresis0) { SetupRising(rising, level); SetupFalling(falling, level); }
    void SetupRising(Hysteresis hyst, float scale = 1) { HYSTERESIS0 = hyst | (std::min(63, int(scale * 64)) << 16) | (std::min(63, int(scale * 64)) << 24); }
    void SetupFalling(Hysteresis hyst, float scale = 1) { HYSTERESIS1 = hyst | (std::min(63, int(scale * 64)) << 16) | (std::min(63, int(scale * 64)) << 24); }
    void Enable() { EFM32_BITSET(CTRL, FlagEnabled); }

    bool Active() const { return GETBIT(STATUS, 0); }
    bool Output() const { return GETBIT(STATUS, 1); }
    bool Conflict() const { return GETBIT(STATUS, 2); }

    void ConfigureOutput(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 0u, 0u); }

    void InterruptEnable() { IEN = ACMP_IEN_EDGE; }
    void InterruptDisable() { IEN = 0; }
    void InterruptClear() { IFC = ~0u; }
    uint32_t InterruptRead() { return IFC; }
    async(WaitForEdge, mono_t timeout = ASYNC_NO_TIMEOUT);

    ALWAYS_INLINE IRQn_Type IRQn() const { ASSERT(Index() == 0); return ACMP0_IRQn; }
    void IRQEnable() { NVIC_EnableIRQ(IRQn()); }
    void IRQDisable() { NVIC_DisableIRQ(IRQn()); }
    void IRQClear() { NVIC_ClearPendingIRQ(IRQn()); }
    void IRQPriority(uint32_t priority) { NVIC_SetPriority(IRQn(), priority); }
    void IRQHandler(Delegate<void> handler) { Cortex_SetIRQHandler(IRQn(), handler); }

private:
    RES_PAIR_DECL(CalculateHysteresis, uint32_t mv);
};

DEFINE_FLAG_ENUM(ACMP::Flags);
