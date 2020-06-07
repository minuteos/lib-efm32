/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-usb/usb/DeviceEndpoints.cpp
 */

#include <usb/DeviceEndpoints.h>
#include <usb/Device.h>

namespace usb
{

void DeviceInEndpoint::ReleaseBuffers()
{
    if (buffer[0])
    {
        free(buffer[0]);
        buffer[0] = buffer[1] = NULL;
        bufferSize = 0;
        usedAny = txAny = 0;
        txBuf = 0;
    }
}

async(DeviceInEndpoint::Configure, const EndpointDescriptor* cfg)
async_def_sync()
{
    if (ep->IsActive())
        ep->Deactivate();

    ReleaseBuffers();

    if (cfg == NULL)
    {
        USBDEBUG("IN(%d) deactivated", ep->Index());
    }
    else
    {
        ep->Activate(cfg->wMaxPacketSize, cfg->type, ep->Index());
        USBDEBUG("IN(%d) MPS %d Type %s", ep->Index(), cfg->wMaxPacketSize, ((const char*[]){ "CTL", "ISO", "BULK", "INT" })[cfg->type]);

        switch ((EndpointType)cfg->type)
        {
            case EndpointType::Bulk:
                // when using maximum packet size, we automatically allocate 19x buffers to be able to fill USB frames
                bufferSize = cfg->wMaxPacketSize * (cfg->wMaxPacketSize == 64 ? 19 : 4);
                break;

            case EndpointType::Interrupt:
            case EndpointType::Isochronous:
                // only one transfer per frame is possible, no point making the buffer bigger
                bufferSize = cfg->wMaxPacketSize;
                break;

            default:
                USBDIAG("  unsupported type");
                async_return(false);
                break;
        }

        USBDIAG("  Using twin %d byte buffers (%d total)", bufferSize, bufferSize * 2);
        buffer[0] = (uint8_t*)malloc(bufferSize * 2);
        buffer[1] = buffer[0] + bufferSize;
        txBuf = 0;
        txAny = usedAny = 0;
        epBuf = -1;
        ep->InterruptEnable();
    }
}
async_end

void DeviceInEndpoint::TransferComplete()
{
    if (epBuf >= 0 && txHalf[epBuf])
    {
        USBEPTRACE(ep->Index() * 2 - 1, buffer[epBuf], txHalf[epBuf]);
        usedHalf[epBuf] = 0;
        packetSent = true;
    }

    epBuf = !epBuf;

    if ((txHalf[epBuf] = usedHalf[epBuf]))
    {
        // continue transmitting from the other half
        ep->TransmitPacket(Span(buffer[epBuf], txHalf[epBuf]));
    }
    else
    {
        epBuf = -1;
    }
}

async(DeviceInEndpoint::Write, Span data, Timeout timeout)
async_def(
    Timeout timeout;
    uint32_t sent;
)
{
    f.timeout = timeout.MakeAbsolute();
    f.sent = 0;

    // writes are always synchronized, to prevent interleaving data from multiple sources
    if (!await_acquire_timeout(lock, 1, f.timeout))
        async_return(0);

    int remaining;
    while ((remaining = data.Length() - f.sent) > 0)
    {
        packetSent = false;

        auto txBuf = this->txBuf;

        int usedBefore = usedHalf[txBuf];
        if (usedBefore)
        {
            if (txHalf[txBuf])  // already started transmitting
            {
                if (!(usedBefore = usedHalf[!txBuf]))
                {
                    // skip to the second buffer
                    this->txBuf = txBuf = !txBuf;
                }
                else
                {
                    // wait for the current transmission to complete if the other buffer is full
                    if (!await_signal_timeout(packetSent, f.timeout))
                        break;
                    else
                        continue;
                }
            }
        }

        if (!usedBefore)
        {
            // clear the transmitting flag
            txHalf[txBuf] = 0;
        }

        int free = bufferSize - usedBefore;
        int block = std::min(remaining, free);
        memcpy(buffer[txBuf] + usedBefore, data.RemoveLeft(f.sent), block);
        usedHalf[txBuf] = usedBefore + block;

        // if there was something in the buffer, sending could have started before we changed the length
        // we need to try again with the other buffer in this case
        if (usedBefore && txHalf[txBuf] == usedBefore)
        {
            this->txBuf = !txBuf;
            continue;
        }

        if (epBuf == -1)
        {
            // start transmitting immediately
            epBuf = txBuf;
            ep->TransmitPacket(Span(buffer[txBuf], txHalf[txBuf] = usedHalf[txBuf]));
        }

        if (block == free)
        {
            // current buffer is full, switch to the other one
            this->txBuf = !txBuf;
        }

        f.sent += block;
    }

    lock = false;
    async_return(f.sent);
}
async_end

void DeviceOutEndpoint::ReleaseBuffers()
{
    if (buffer[0])
    {
        free(buffer[0]);
        buffer[0] = buffer[1] = rx = NULL;
        bufferSize = 0;
        usedAny = 0;
    }
}

async(DeviceOutEndpoint::Configure, const EndpointDescriptor* cfg)
async_def_sync()
{
    if (ep->IsActive())
        ep->Deactivate();

    ReleaseBuffers();

    if (cfg == NULL)
    {
        USBDEBUG("OUT(%d) deactivated", ep->Index());
    }
    else
    {
        ep->Activate(cfg->wMaxPacketSize, cfg->type);
        USBDEBUG("OUT(%d) MPS %d Type %s", ep->Index(), cfg->wMaxPacketSize, ((const char*[]){ "CTL", "ISO", "BULK", "INT" })[cfg->type]);

        switch ((EndpointType)cfg->type)
        {
            case EndpointType::Bulk:
                // there is no need to use extreme bulk buffers for RX, as the RX FIFO captures any incoming data anyway
                bufferSize = cfg->wMaxPacketSize * 4;
                break;

            case EndpointType::Interrupt:
            case EndpointType::Isochronous:
                // only one transfer per frame is possible, no point making the buffer bigger
                bufferSize = cfg->wMaxPacketSize;
                break;

            default:
                USBDIAG("  unsupported type");
                async_return(false);
        }
        USBDIAG("  Using twin %d byte buffers (%d total)", bufferSize, bufferSize * 2);
        rx = buffer[0] = (uint8_t*)malloc(bufferSize * 2);
        buffer[1] = buffer[0] + bufferSize;
        // start receiving
        ep->ReceivePacket(Buffer(buffer[0], bufferSize));
        ep->InterruptEnable();
        epBuf = 0;
    }
}
async_end

void DeviceOutEndpoint::TransferComplete()
{
    if ((usedHalf[epBuf] = bufferSize - ep->ReceivedLength()))
    {
        USBEPTRACE(ep->Index() * 2, buffer[epBuf], usedHalf[epBuf]);
        newData = true;
        epBuf = !epBuf;
    }

    if (usedHalf[epBuf])
    {
        epBuf = -1;
    }
    else
    {
        // continue receiving if there is a free buffer
        ep->ReceivePacket(Buffer(buffer[epBuf], bufferSize));
    }
}

async(DeviceOutEndpoint::Read, Buffer data, Timeout timeout)
async_def(
    Timeout timeout;
)
{
    newData = false;
    f.timeout = timeout.MakeAbsolute();

    while (!usedAny)
    {
        if (!await_signal_until(newData, f.timeout))
            async_return(0);

        newData = false;
    }

    unsigned read = 0;
    int remaining;
    while ((remaining = data.Length() - read) > 0)
    {
        bool half = rx >= buffer[1];
        uint32_t used = usedHalf[half];
        if (!used)
        {
            // no more data remaining
            ASSERT(read);   // there must have been at least something
            break;
        }
        unsigned avail = buffer[half] + used - rx;
        if (avail > (unsigned)remaining)
        {
            // partial read
            memcpy(data.Pointer() + read, rx, remaining);
            rx += remaining;
            read += remaining;
            break;
        }
        else
        {
            // full read, move to the other half
            memcpy(data.Pointer() + read, rx, avail);
            rx = buffer[!half];
            read += avail;
            usedHalf[half] = 0;
            if (epBuf == -1)
            {
                epBuf = half;
                ep->ReceivePacket(Buffer(buffer[epBuf], bufferSize));
            }
        }
    }

    async_return(read);
}
async_end

}
