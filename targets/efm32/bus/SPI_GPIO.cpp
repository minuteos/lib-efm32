/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/bus/SPI_GPIO.cpp
 */

#if EFM32_USE_GPIO_SPI

#include "SPI_GPIO.h"

namespace bus
{

async(SPI_GPIO::Transfer, Descriptor* descriptors, size_t count)
async_def_sync()
{
    cs.Res();
    while (count--)
    {
        switch (descriptors->op)
        {
            case Descriptor::Read:
                for (size_t i = 0; i < descriptors->size; i++)
                    descriptors->data[i] = ReadByte();
                break;
            case Descriptor::ReadSame:
                for (size_t i = 0; i < descriptors->size; i++)
                    descriptors->data[0] = ReadByte();
                break;
            case Descriptor::Write:
                for (size_t i = 0; i < descriptors->size; i++)
                    WriteByte(descriptors->data[i]);
                break;
            case Descriptor::WriteSame:
                for (size_t i = 0; i < descriptors->size; i++)
                    WriteByte(descriptors->data[0]);
                break;
        }
        descriptors++;
    }
    cs.Set();
}
async_end

static ALWAYS_INLINE void clockdelay()
{
    __NOP(); __NOP();
}

uint8_t SPI_GPIO::ReadByte()
{
    int res = 0;
    for (int i = 0; i < 8; i++)
    {
        clockdelay();
        clk.Toggle();
        clockdelay();
        res = (res << 1) | miso;
        clk.Toggle();
    }
    return res;
}

void SPI_GPIO::WriteByte(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        mosi.Set(GETBIT(byte, i));
        clockdelay();
        clk.Toggle();
        clockdelay();
        clk.Toggle();
    }
}

}

#endif
