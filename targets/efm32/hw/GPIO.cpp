/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/GPIO.cpp
 */

#include <hw/GPIO.h>
#include <hw/CMU.h>

const char* GPIOPin::Name() const
{
    if (!mask)
        return "Px";

    static char tmp[4];
    int index = Index();
    tmp[0] = 'A' + Port().Index();
    if (index < 10)
    {
        tmp[1] = '0' + index;
        tmp[2] = 0;
    }
    else
    {
        tmp[1] = '0' + index / 10;
        tmp[2] = '0' + index % 10;
        tmp[3] = 0;
    }
    return tmp;
}

int GPIOPin::GetLocation(GPIOLocations_t locations, GPIOPinID id)
{
    for (auto i = 0; locations[i]; i++)
        if (locations[i] == id)
            return i;
    return -1;
}

void GPIOPort::Configure(uint32_t mask, GPIOPin::Mode mode)
{
    if (!mask)
        return;

    CMU->EnableGPIO();

#if EFM32_GPIO_DRIVE_CONTROL
    if (!(MODEL || MODEH) && CTRL == EFM32_GPIO_DRIVE_RESET)
        Setup();	// configure port drive settings, unless they were already modified
#endif

    unsigned modeNum = mode & GPIOPin::ModeMask;

    DBGC("gpio", "Configuring port %s (mode %d", GPIOPin(this, mask).Name(), modeNum);

    if (mode & GPIOPin::FlagAltDrive)
    {
        ASSERT(GETBIT(0xF10, modeNum));	// modes that can have alternate drive strength - 4, 8-11
        if ((mode & GPIOPin::ModeMask) == GPIOPin::PushPull)
            modeNum++;	// 4 -> 5
        else
            modeNum |= 4;	// 8-11 -> 12-15

        _DBG(" + alt");
    }

    if (mode & GPIOPin::FlagFilter)
    {
        if (modeNum == 1)
        {
            // special case, Input mode enables filter by setting DOUT
            DOUT |= mask;
            _DBG(", filter");
            goto filter_set;
        }
        else
        {
            ASSERT(GETBIT(0x5504, modeNum));	// modes that can have glitch filter enabled - 2, 8, 10, 12, 14
            modeNum++;
        }
        _DBG(" + filter");
    }

    if (mode & (GPIOPin::FlagAltDrive | GPIOPin::FlagFilter))
    {
        _DBG(" = %d", modeNum);
    }

    if (mode & GPIOPin::FlagSet)
    {
        DOUT |= mask;
        _DBG(", set");
    }
    else
    {
        DOUT &= ~mask;
    }
filter_set:
#ifdef _GPIO_P_OVTDIS_MASK
    if (mode & GPIOPin::FlagNoOvervoltage)
    {
        OVTDIS |= mask;
        _DBG(", no overvoltage");
    }
    else
    {
        OVTDIS &= ~mask;
    }
#endif

    _DBG(")\n");

    do
    {
        unsigned bit = __builtin_ctz(mask);
        RESBIT(mask, bit);
        unsigned shift = (bit & 7) * 4;
        if (bit > 7)
            MODMASK(MODEH, MASK(4) << shift, modeNum << shift);
        else
            MODMASK(MODEL, MASK(4) << shift, modeNum << shift);
    } while(mask);
}

#ifdef EFM32_GPIO_LINEAR_INDEX

void GPIOPort::ConfigureAlternate(AltSpec spec, volatile uint32_t& routepen, unsigned locOffset)
{
    if (spec.pin > 15)
        return;

    unsigned n = (spec.pin - locOffset + EFM32_GPIO_LINEAR_INDEX[GetIndex(this)]) & 31;

    CMU->EnableGPIO();

    DBG("gpio: Routing port %c%d to peripheral %08X route %d location %d\n", 'A' + Index(), spec.pin, ((uint)&routepen) & ~(MASK(10)), spec.loc, n);

    volatile uint32_t* ploc = &routepen + 1 + (spec.loc >> 2);
    unsigned loc = (spec.loc & 3) << 3;
    MODMASK(*ploc, MASK(6) << loc, n << loc);
    SETBIT(routepen, spec.route);
    return Configure(BIT(spec.pin), spec.mode);
}

#endif

uint32_t GPIOBlock::EnableInterrupt(GPIOPinID id, unsigned risingFalling)
{
    ASSERT(id.Port() >= 0);
    unsigned port = id.Port();

    unsigned pin = id.Pin();
    unsigned block = pin & ~3;

    for (unsigned n = block; n < block + 4; n++)
    {
        uint32_t mask = BIT(n);
        if (!(IEN & mask))
        {
            if (risingFalling & 1)
                EXTIRISE |= mask;
            if (risingFalling & 2)
                EXTIFALL |= mask;

            unsigned h = n >> 3;
            unsigned offset = (n & 7) << 2;
            MODMASK(*(&EXTIPSELL + h), 15 << offset, port << offset);
            MODMASK(*(&EXTIPINSELL + h), 15 << offset, (pin & 3) << offset);
            IFC = mask;
            IEN |= mask;

            auto irq = (n & 1) ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn;
            Cortex_SetIRQWakeup(irq);
            NVIC_ClearPendingIRQ(irq);
            NVIC_EnableIRQ(irq);
            return mask;
        }
    }

    DBGCL("gpio", "No free external interrupts in block %d-%d", block >> 2, (block >> 2) + 3);
    ASSERT(0);
    return 0;
}

void GPIOBlock::DisableInterrupt(uint32_t mask)
{
    if (IEN & mask)
    {
        IEN &= ~mask;
        EXTIRISE &= ~mask;
        EXTIFALL &= ~mask;
        IFC = mask;
        auto irq = !(__builtin_clz(mask) & 1) ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn;
        NVIC_ClearPendingIRQ(irq);
    }
}

#ifdef Ckernel
async(GPIOPort::WaitFor, uint32_t indexAndState, mono_t until)
async_def(
    uint8_t index;
    bool state;
    uint32_t intMask;
)
{
    f.index = indexAndState & 0xF;
    f.state = indexAndState & 0x10;
    if (GETBIT(DIN, f.index) == f.state)
    {
        async_return(true);
    }

    f.intMask = GPIO->EnableInterrupt(GPIOPinID(Index(), f.index), 2 >> f.state);
    bool result;
    if (until)
    {
        result = await_mask_until(DIN, BIT(f.index), BIT(f.index) * f.state, until);
    }
    else
    {
        result = await_mask(DIN, BIT(f.index), BIT(f.index) * f.state);
    }
    GPIO->DisableInterrupt(f.intMask);
    async_return(result);
}
async_end
#endif

