/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-usb/usb/Packets.h
 * 
 * Standard USB packets
 */

#include <base/base.h>

#include <usb/Descriptors.h>

namespace usb
{

PACKED_STRUCT SetupPacket
{
    enum Direction
    {
        DirOut,
        DirIn,
    };

    enum Type
    {
        TypeStandard,
        TypeClass,
        TypeVendor,
        TypeInvalid,
    };

    enum Recipient
    {
        RecipientDevice,
        RecipientInterface,
        RecipientEndpoint,
        RecipientOther,
    };

    enum Request : uint8_t
    {
        StdGetStatus = 0,
        StdClearFeature = 1,
        StdSetFeature = 3,
        StdSetAddress = 5,
        StdGetDescriptor = 6,
        StdSetDescriptor = 7,
        StdGetConfiguration = 8,
        StdSetConfiguration = 9,
    };

    enum Feature
    {
        FeatureEndpointHalt = 0,
        FeatureDeviceRemoteWakeup = 1,
    };

    union
    {
        uint32_t w[2];
        struct
        {
            union
            {
                uint8_t bmRequestType;
                struct
                {
                    Recipient recipient : 5;
                    Type type : 2;
                    Direction direction : 1;
                };
            };
            Request bRequest;
            union
            {
                uint16_t wValue;
                struct
                {
                    uint8_t descriptorIndex;
                    DescriptorType descriptorType;
                };
            };
            uint16_t wIndex;
            uint16_t wLength;
        };
    };
};

}
