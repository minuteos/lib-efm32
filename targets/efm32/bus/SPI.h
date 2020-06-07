/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/bus/SPI.h
 */

#pragma once

#include <base/base.h>

#include <hw/USART.h>

namespace bus
{

class SPI
{
    ::USART& usart;

public:
    constexpr SPI(USART& usart)
        : usart(usart) {}
    constexpr SPI(USART* usart)
        : usart(*usart) {}

    //! Opaque handle representing the ChipSelect signal
    class ChipSelect
    {
        constexpr ChipSelect(unsigned loc)
            : loc(loc) {}

        uint8_t loc;

        friend class SPI;
    };

    //! Represents an single SPI transfer step
    using Descriptor = USART::SyncTransferDescriptor;

    //! Retrieves a ChipSelect handle for the specified GPIO pin for the current USART
    ChipSelect GetChipSelect(GPIOPin pin) { return ChipSelect(usart.GetCsLocation(pin)); }
    //! Acquires the bus for the device identified by the specified @ref ChipSelect
    async(Acquire, ChipSelect cs, Timeout timeout = Timeout::Infinite) { return async_forward(usart.BindCs, cs.loc, timeout); }
    //! Releases the bus
    void Release() { usart.ReleaseCs(); }

    //! Performs a single SPI transfer
    async(Transfer, Descriptor& descriptor) { return async_forward(usart.SyncTransfer, &descriptor, 1); }
    //! Performs a chain of SPI transfers
    async(Transfer, Descriptor* descriptors, size_t count) { return async_forward(usart.SyncTransfer, descriptors, count); }
    //! Performs a chain of SPI transfers
    template<size_t n> ALWAYS_INLINE async(Transfer, Descriptor (&descriptors)[n]) { return async_forward(usart.SyncTransfer, descriptors, n); }
};

}
