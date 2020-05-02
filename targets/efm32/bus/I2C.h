/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * lib-efm32/targets/efm32/bus/I2C.h
 */

#pragma once

#include <base/base.h>

#include <hw/I2C.h>

namespace bus
{

class I2C
{
    ::I2C& i2c;

public:
    constexpr I2C(::I2C& i2c)
        : i2c(i2c) {}
    constexpr I2C(::I2C* i2c)
        : i2c(*i2c) {}

    //! Starts or restarts a read transaction, reading the specified number of bytes from the bus
    async(Read, uint8_t address, Buffer data, bool start, bool stop) { return async_forward(i2c.Read, address, data, start, stop); }
    //! Continues a read transaction, reading the specified number of bytes from the bus
    async(Read, Buffer data, bool stop) { return async_forward(i2c.Read, data, stop); }

    //! Starts or restarts a write transaction, writing the specified number of bytes to the bus
    async(Write, uint8_t address, Span data, bool start, bool stop) { return async_forward(i2c.Write, address, data, start, stop); }
    //! Continues a write transaction, writing the specified number of bytes to the bus
    async(Write, Span data, bool stop) { return async_forward(i2c.Write, data, stop); }
};

}
