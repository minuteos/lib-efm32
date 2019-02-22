/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-usb/usb/Descriptors.cpp
 */

#include <usb/Descriptors.h>

namespace usb
{

const EndpointDescriptor* ConfigDescriptorHeader::FindEndpoint(uint8_t address, int interface, int alternate) const
{
    const DescriptorHeader* end = End();
    bool active = false;

    for (const DescriptorHeader* hdr = this; hdr < end; hdr = hdr->Next())
    {
        switch (hdr->bDescriptorType)
        {
            case DescriptorType::Interface:
            {
                auto ifd = (const InterfaceDescriptorHeader*)hdr;
                active = (interface < 0 || interface == ifd->bInterfaceNumber) &&
                    (alternate < 0 || alternate == ifd->bAlternateSetting);
                break;
            }

            case DescriptorType::Endpoint:
                if (active)
                {
                    auto epd = (const EndpointDescriptor*)hdr;
                    if (epd->bEndpointAddress == address)
                        return epd;
                }
                break;

            default:
                break;
        }
    }

    return NULL;
}

}
