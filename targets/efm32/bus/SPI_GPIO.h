/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/bus/SPI_GPIO.h
 *
 * A minimal bus::SPI implementation using GPIO
 */

#include <base/base.h>
#include <base/Span.h>

#include <hw/GPIO.h>

namespace bus
{

class SPI_GPIO
{
    GPIOPin clk, mosi, miso, cs;

    class Descriptor
    {
        uint8_t* data;
        size_t size;
        enum { Read, ReadSame, Write, WriteSame } op;

    public:
        void Receive(Buffer buf) { data = (uint8_t*)buf.Pointer(); size = buf.Length(); op = Read; }
        void ReceiveSame(volatile void* ptr, size_t len) { data = (uint8_t*)ptr; size = len; op = ReadSame; }
        void Transmit(Span buf) { data = (uint8_t*)buf.Pointer(); size = buf.Length(); op = Write; }
        void TransmitSame(const uint8_t* value, size_t len) { data = (uint8_t*)value; size = len; op = WriteSame; }
        constexpr size_t Length() const { return size; }

        friend class SPI_GPIO;
    };

    async(Transfer, Descriptor* descriptors, size_t count);
    uint8_t ReadByte();
    void WriteByte(uint8_t b);

public:
    SPI_GPIO(GPIOPin clk, GPIOPin mosi, GPIOPin miso)
        : clk(clk), mosi(mosi), miso(miso), cs(Px) {}

    friend class SPI;
};

class SPI
{
    SPI_GPIO& bus;

public:
    SPI(SPI_GPIO& bus)
        : bus(bus) {}

    using ChipSelect = GPIOPin;
    using Descriptor = SPI_GPIO::Descriptor;

    static GPIOPin GetChipSelect(ChipSelect cs) { return cs; }
    static constexpr size_t MaximumTransferSize() { return ~0u; }

    async(Acquire, ChipSelect cs) async_def_sync() { bus.cs = cs; async_return(true); } async_end
    void Release() {}

    async(Transfer, Descriptor& descriptor) { return async_forward(Transfer, &descriptor, 1); }
    async(Transfer, Descriptor* descriptors, size_t count) { return async_forward(bus.Transfer, descriptors, count); }
    template<size_t n> ALWAYS_INLINE async(Transfer, Descriptor (&descriptors)[n]) { return async_forward(Transfer, descriptors, n); }
};

}
