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
