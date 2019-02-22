/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-usb/usb/DeviceEndpoints.h
 */

#pragma once

#include <kernel/kernel.h>
#include <io/io.h>

#include <usb/Descriptors.h>

#include <hw/USB.h>

namespace usb
{

class DeviceInEndpoint : public io::OutputStream
{
    friend class Device;

    class Device* owner;
    USBInEndpoint* ep;
    uint8_t* buffer[2] = {};
    uint32_t bufferSize = 0;
    union
    {
        volatile uint32_t txAny = 0;
        volatile uint16_t txHalf[2];
    };
    union
    {
        volatile uint32_t usedAny = 0;
        volatile uint16_t usedHalf[2];
    };
    int8_t epBuf = -1;
    int8_t txBuf = 0;
    bool lock;
    bool packetSent;

    async(Configure, const EndpointDescriptor* config);

    void ReleaseBuffers();
    void TransferComplete();

public:
    virtual async(Write, Span data, unsigned msTimeout = 0);
};

class DeviceOutEndpoint : public io::InputStream
{
    friend class Device;

    class Device* owner;
    USBOutEndpoint* ep;
    uint8_t* buffer[2] = {};
    uint8_t* rx = NULL;
    uint32_t bufferSize = 0;
    union
    {
        volatile uint32_t usedAny = 0;
        volatile uint16_t usedHalf[2];
    };
    int epBuf = -1;
    bool newData;

    async(Configure, const EndpointDescriptor* config);

    void ReleaseBuffers();
    void TransferComplete();

public:
    virtual async(Read, Buffer buffer, unsigned msTimeout = 0);
};

}
