/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-usb/usb/Device.cpp
 */

#include <usb/Device.h>

#include <hw/USB.h>

namespace usb
{

async(Device::Task)
async_def()
{
    // initialize the endpoint structures
    for (unsigned i = 1; i <= USB_IN_ENDPOINTS; i++)
    {
        auto& ep = In(i);
        ep.owner = this;
        ep.ep = &usb->In(i);
    }

    for (unsigned i = 1; i <= USB_OUT_ENDPOINTS; i++)
    {
        auto& ep = Out(i);
        ep.owner = this;
        ep.ep = &usb->Out(i);
    }

    usb->IRQDisable();
    usb->IRQHandler(GetDelegate(this, &Device::IRQHandler));

    await(usb->CoreEnable);
    PLATFORM_DEEP_SLEEP_DISABLE();  // no more sleeping while USB is active
    await(usb->CoreReset);

    usb->ForceDevice();
    async_delay_ms(30);

    // 37.4.1.2 EFM322GG11-rm
    // configure device (Full speed, 80% periodic frame interval)
    usb->DeviceSetup(_USB::SpeedFull | _USB::PeriodicFrame80 | _USB::NonZeroLengthStatusOutHandshake);
    usb->DeviceThresholdEnable(_USB::ThresholdTx);
    usb->DeviceConnect();
    USBDEBUG("Device ready for connection");

    EnableInitInterrupts();
    usb->IRQClear();
    usb->IRQEnable();

    // wait for reset interrupt
    for (;;)
    {
        await_signal(tasks);
        auto todo = kernel::AtomicExchange(tasks, Tasks::None);

        if (!!(todo & Tasks::Control))
            await(HandleControl);
    }
}
async_end

void Device::AllocateFifo()
{
    // we are using thresholding, maximum TX fifo size is 16 words (MPS)
    // since there are only 7 TX fifos (112 words), there are still 400 words
    // remaining. We leave 16 words for control structures, with 384 words for RX.

    constexpr unsigned rxWords = 64, txWords = 16;
    usb->RxFifoSetup(rxWords);
    usb->TxFifo0Setup(rxWords, txWords);

    for (unsigned i = 1, ptr = rxWords + txWords; i <= USB_IN_ENDPOINTS; i++, ptr += txWords)
    {
        usb->TxFifoSetup(i, ptr, txWords);
    }
}

void Device::IRQHandler()
{
    UNUSED auto sys = usb->IFC, core = usb->GINTSTS;
    uint32_t handled = 0;

    // clear all interrupts that will be handled
    core &= usb->GINTMSK;
    usb->GINTSTS = core;

    if (core & (USB_GINTSTS_USBRST | USB_GINTSTS_WKUPINT | USB_GINTSTS_IEPINT | USB_GINTSTS_OEPINT))
    {
        handled |= USB_GINTSTS_WKUPINT;

        if (suspended)
            USBDEBUG("WAKEUP");
        suspended = false;
    }
    else if (core & USB_GINTSTS_USBSUSP)
    {
        handled |= USB_GINTSTS_USBSUSP;

        if (!suspended)
            USBDEBUG("SUSPEND");
        suspended = true;
    }

    if (core & USB_GINTSTS_USBRST)
    {
        handled |= USB_GINTSTS_USBRST;
        state = State::Default;
        USBDEBUG("RESET");
        usb->DeviceAddress(0);

        // 37.4.4.1.1 EFM32GG11-rm

        //  1. reset all endpoints
        for (unsigned i = 0; i <= USB_IN_ENDPOINTS; i++)
            usb->In(i).Reset();
        for (unsigned i = 0; i <= USB_OUT_ENDPOINTS; i++)
            usb->Out(i).Reset();

        //  2. configure endpoint interrupts for SETUP
        usb->DAINTMSK |= USB_DAINTMSK_INEPMSK0 | USB_DAINTMSK_OUTEPMSK0;
        usb->DOEPMSK |= USB_DOEPMSK_SETUPMSK | USB_DOEPMSK_XFERCOMPLMSK;
        usb->DIEPMSK |= USB_DIEPMSK_XFERCOMPLMSK | USB_DIEPMSK_TIMEOUTMSK;

        //  3. mask slave mode interrupts - not applicable

        //  4. configure FIFO
        AllocateFifo();

        //  5. configure OUT 0 for SETUP
        ControlSetup();
    }

    if (core & USB_GINTSTS_ENUMDONE)
    {
        // enumeration done
        handled |= USB_GINTSTS_ENUMDONE;
        fullSpeed = usb->DeviceFullSpeed();
        state = State::Default;
        USBDEBUG("ENUMDONE: %s Speed", fullSpeed ? "Full" : "Low");

        EnableAllInterrupts();
    }

    if (core & USB_GINTSTS_IEPINT)
    {
        handled |= USB_GINTSTS_IEPINT;
        HandleIn();
    }

    if (core & USB_GINTSTS_OEPINT)
    {
        handled |= USB_GINTSTS_OEPINT;
        HandleOut();
    }

    if (core &= ~handled)
    {
        USBDEBUG("UNHANDLED INTERRUPT: %08X", core);
    }
}

inline void Device::HandleOut()
{
    auto mask = usb->DeviceEndpointInterrupts();

    if (mask & USB_DAINT_OUTEPINT0)
    {
        // endpoint 0 is handled separately
        HandleOutControl();
    }

    for (unsigned i = 1; i <= USB_OUT_ENDPOINTS; i++)
    {
        if (!(mask & (USB_DAINT_OUTEPINT0 << i)))
            continue;

        auto& ep = Out(i);
        auto status = ep.ep->InterruptClear();

        USBDIAG("EP OUT(%d) INT %08X", i, status);

        if (status & USB_DOEP_INT_XFERCOMPL)
        {
            ep.TransferComplete();
        }
    }
}

inline void Device::HandleIn()
{
    auto mask = usb->DeviceEndpointInterrupts();

    if (mask & USB_DAINT_INEPINT0)
    {
        HandleInControl();
    }

    for (unsigned i = 1; i <= USB_IN_ENDPOINTS; i++)
    {
        if (!(mask & (USB_DAINT_INEPINT0 << i)))
            continue;

        auto& ep = In(i);
        auto status = ep.ep->InterruptClear();

        USBDIAG("EP IN(%d) INT %08X", i, status);

        if (status & USB_DIEP_INT_XFERCOMPL)
        {
            ep.TransferComplete();
        }
    }
}

inline void Device::HandleOutControl()
{
    auto& ep = usb->Out(0);
    auto status = ep.InterruptClear();

    USBDIAG("EP OUT(0) INT %08X", status);

    if (status & USB_DOEP_INT_SETUP)
    {
        SetupPacket pkt;

        if (status & USB_DOEP_INT_BACK2BACKSETUP)
        {
            // setup calculated from DMAADDR
            pkt = ((SetupPacket*)ep.Pointer())[-1];
        }
        else
        {
            // setup calculated from counter
            auto n = ep.SetupCount();
            pkt = ctrl.setupBuffer[n >= countof(ctrl.setupBuffer) ? 0 : countof(ctrl.setupBuffer) - 1 - n];
        }

        ctrl.setup = pkt;
        if (pkt.direction == pkt.DirOut && pkt.wLength)
        {
            // more data will follow, however
            // we can accept at most one full packet
            if (pkt.wLength > sizeof(ctrl.data))
            {
                // data too long, not supported
                USBDEBUG("!!! CONTROL request too long: %d > %d", pkt.wLength, sizeof(ctrl.data));
                ControlStall();
            }
            else
            {
                ep.ReceivePacket(ctrl.data);
                ctrl.state = ControlState::DataRx;
            }
        }
        else
        {
            ctrl.state = ControlState::Processing;
            tasks = tasks | Tasks::Control;
        }
    }
    else if (status & USB_DOEP_INT_XFERCOMPL)
    {
        switch (ctrl.state)
        {
        case ControlState::DataRx:
        {
            USBDIAG("EP0 data received: %H", Span(ctrl.data, ep.PacketSize() - ep.ReceivedLength()));
            if (ep.PacketSize() - ep.ReceivedLength() != ctrl.setup.wLength)
            {
                USBDEBUG("!!! Invalid SETUP OUT data length %d != %d", ep.PacketSize() - ep.ReceivedLength(), ctrl.setup.wLength);
                ControlStall();
            }
            else
            {
                ctrl.state = ControlState::Processing;
                tasks = tasks | Tasks::Control;
            }

            ctrl.state = ControlState::Processing;
            tasks = tasks | Tasks::Control;
            break;
        }

        case ControlState::StatusRx:
            USBDIAG("EP0 data confirmed");
            if (ep.ReceivedLength() != ep.PacketSize())
            {
                USBDEBUG("!!! Data received in STATUS: %H", Span(ctrl.data, ep.PacketSize() - ep.ReceivedLength()));
            }
            ControlSetup();
            break;

        default:
            break;
        }
    }
}

inline void Device::HandleInControl()
{
    // handle endpoint 0 separately
    auto& ep = usb->In(0);
    auto status = ep.InterruptClear();

    USBDIAG("EP IN(0) CTL %08X INT %08X", ep.CTL, status);

    if (status & USB_DIEP_INT_XFERCOMPL)
    {
        switch (ctrl.state)
        {
            case ControlState::StatusTx:
                // prepare for next SETUP packet
                USBDIAG("Control ACK sent, waiting for next SETUP");
                ControlSetup();
                break;

            case ControlState::DataTx:
                if (ctrl.txRemain.Length())
                {
                    // continue transmitting
                    ep.TransmitPacket(ctrl.txRemain.ConsumeLeft(ep.PacketSize()));
                }
                else
                {
                    USBDIAG("Control data transmitted, waiting for STATUS");
                    usb->Out(0).ReceivePacket(ctrl.data);
                    ctrl.state = ControlState::StatusRx;
                }
                break;

            default:
                break;
        }
    }
}

void Device::ControlSuccess(Span data)
{
    if (ctrl.hasResult)
    {
        USBDEBUG("!!! ControlResult() called multiple times");
        return;
    }

    ctrl.hasResult = true;
    ASSERT(ctrl.state == ControlState::Processing);

    if (ctrl.setup.direction == SetupPacket::DirOut)
    {
        if (data.Length())
        {
            USBDEBUG("!!! ControlResult() data providen in OUT (H2D) direction: %H", data);
            data = Span();
        }
        ctrl.state = ControlState::StatusTx;
    }
    else
    {
        // cut the data to the requested length
        data = data.Left(ctrl.setup.wLength);

        // data fitting in one packet is copied in the control data buffer
        // it is up to the application to provide a peristent location for data
        // longer than that
        if (data.Length() <= sizeof(ctrl.data))
        {
            data = data.CopyTo(ctrl.data);
        }

        auto send = data.ConsumeLeft(usb->In(0).PacketSize());
        ctrl.txRemain = data;
        ctrl.state = ControlState::DataTx;
        data = send;
    }

    usb->In(0).TransmitPacket(data);
    if (data.Length())
        USBDIAG("CONTROL %08X >> %H", usb->In(0).CTL, data);
    else
        USBDIAG("CONTROL %08X ACK", usb->In(0).CTL);
}

async(Device::HandleControl)
async_def()
{
    auto setup = ctrl.setup;
    ctrl.hasResult = false;

    switch (setup.type)
    {
        case SetupPacket::TypeStandard:
            switch (setup.bRequest)
            {
                case SetupPacket::StdGetStatus: HandleControlGetStatus(setup); break;
                case SetupPacket::StdClearFeature: HandleControlFeature(setup, false); break;
                case SetupPacket::StdSetFeature: HandleControlFeature(setup, true); break;
                case SetupPacket::StdSetAddress: HandleControlSetAddress(setup); break;
                case SetupPacket::StdGetDescriptor: HandleControlGetDescriptor(setup); break;
                case SetupPacket::StdGetConfiguration: HandleControlGetConfiguration(setup); break;
                case SetupPacket::StdSetConfiguration: await(HandleControlSetConfiguration, setup); break;
                default: break;
            }
            break;

        default:
            if (ctrl.setup.direction == SetupPacket::DirOut)
                ctrl.callback(ctrl.setup, Span(ctrl.data, ctrl.setup.wLength));
            else
                ctrl.callback(ctrl.setup, Span());
            break;
    }

    if (!ctrl.hasResult)
    {
        USBDEBUG("!!! CONTROL %H %H unsupported", Span(setup), Span(ctrl.data, setup.wLength));
        ControlStall();
    }
}
async_end

void Device::HandleControlGetStatus(SetupPacket setup)
{
    if (setup.direction == SetupPacket::DirIn && !setup.wValue && setup.wLength == 2)
    {
        switch (setup.recipient)
        {
            case SetupPacket::RecipientDevice:
                if (setup.wIndex == 0)
                {
                    if (config)
                    {
                        ctrl.data16 = !!(config->bmAttributes & ConfigAttributes::SelfPowered) * BIT(0) |
                            !!(config->bmAttributes & ConfigAttributes::RemoteWakeup) * BIT(1);
                    }
                    else
                    {
                        ctrl.data16 = 0;
                    }
                    ControlSuccess(ctrl.data16);
                    return;
                }
                break;

            default:
                break;
        }
    }

    USBDEBUG("!!! GET_STATUS packet corrupted");
}

void Device::HandleControlFeature(SetupPacket setup, bool set)
{
    if (setup.wLength == 0)
    {
        switch (setup.recipient)
        {
            case SetupPacket::RecipientDevice:
                if (setup.wIndex != 0)
                    break;

                if (!(config && state == State::Configured))
                    return; // not supported right now

                if (setup.wValue == SetupPacket::FeatureDeviceRemoteWakeup)
                {
                    if (!!(config->bmAttributes & ConfigAttributes::RemoteWakeup))
                    {
                        USBDIAG("Feature REMOTE_WAKEUP %d", set);
                        remoteWakeupEnabled = set;
                        ControlSuccess();
                    }
                }
                return;

            case SetupPacket::RecipientEndpoint:
            {
                uint num = setup.wIndex & 0x7F;
                bool in = setup.wIndex & 0x80;
                if (num == 0)
                    return; // no features on EP0

                if (num > (in ? USB_IN_ENDPOINTS : USB_OUT_ENDPOINTS))
                    return; // endpoint number out of range

                if (setup.wValue == SetupPacket::FeatureEndpointHalt)
                {
                    USBDIAG("%s(%d) STALL: %d", in ? "IN" : "OUT", num, set);

                    if (in)
                        usb->In(num).Stall(set);
                    else
                        usb->Out(num).Stall(set);
                    ControlSuccess();
                }
                return;
            }

            default:
                // unsupported recipient
                return;
        }
    }

    USBDEBUG("!!! %s_FEATURE packet corrupted", set ? "SET" : "CLEAR");
}

void Device::HandleControlSetAddress(SetupPacket setup)
{
    if (setup.recipient != SetupPacket::RecipientDevice ||
        setup.direction != SetupPacket::DirOut ||
        setup.wIndex || setup.wLength || setup.wValue > 127)
    {
        USBDEBUG("!!! SET_ADDRESS packet corrupted");
        return;
    }

    USBDIAG("SET_ADDRESS %04X", setup.wValue);
    usb->DeviceAddress(setup.wValue);
    if (state == State::Default && setup.wValue)
        state = State::Addressed;
    else if (state == State::Addressed && !setup.wValue)
        state = State::Default;

    ControlSuccess();
}

void Device::HandleControlGetDescriptor(SetupPacket setup)
{
    USBDIAG("GET_DESCRIPTOR %04X ID %d len %d", setup.wValue, setup.wIndex, setup.wLength);
    if (setup.recipient != SetupPacket::RecipientDevice ||
        setup.direction != SetupPacket::DirIn)
    {
        USBDEBUG("!!! GET_DESCRIPTOR packet corrupted");
        return;
    }

    Span data;
    switch (setup.descriptorType)
    {
        case DescriptorType::Device:
            if (setup.descriptorIndex != 0)
            {
                USBDEBUG("!!! GET_DEVICE_DESCRIPTOR %d > 0", setup.descriptorIndex);
                return;
            }

            data = deviceDescriptor;
            break;

        case DescriptorType::Config:
        {
            if (setup.descriptorIndex >= configDescriptorCount)
            {
                USBDEBUG("!!! GET_CONFIG_DESCRIPTOR %d >= %d", setup.descriptorIndex, configDescriptorCount);
                return;
            }

            auto desc = configDescriptors[setup.descriptorIndex];
            data = Span(desc, desc->wTotalLength);
            break;
        }

        case DescriptorType::String:
        {
            auto desc = stringDescriptors;
            int nValid = 0;

            while (desc && nValid < setup.descriptorIndex)
            {
                desc = desc->Next();
                if (desc->len)
                    nValid++;
                else
                    desc = NULL;
            }

            if (!desc)
            {
                USBDEBUG("!!! GET_STRING_DESCRIPTOR %d >= %d", setup.descriptorIndex, nValid);
                return;
            }

            data = Span(desc, desc->len);
            break;
        }

        default:
            break;
    }

    ControlSuccess(data);
}

void Device::HandleControlGetConfiguration(SetupPacket setup)
{
    USBDIAG("GET_CONFIGURATION");
    if (setup.recipient != SetupPacket::RecipientDevice ||
        setup.direction != SetupPacket::DirIn ||
        setup.wIndex || setup.wValue || setup.wLength != 1)
    {
        USBDEBUG("!!! GET_CONFIGURATION packet corrupted");
        return;
    }

    ctrl.data[0] = config ? config->bConfigurationValue : 0;
    ControlSuccess(ctrl.data[0]);
}

async(Device::HandleControlSetConfiguration, SetupPacket setup)
async_def()
{
    USBDIAG("SET_CONFIGURATION %d", setup.wValue);
    if (setup.recipient != SetupPacket::RecipientDevice ||
        setup.wIndex || setup.wLength || setup.wValue > 255)
    {
        USBDEBUG("!!! SET_CONFIGURATION packet corrupted");
        async_return(false);
    }

    if (setup.wValue && (state == State::Addressed || state == State::Configured))
    {
        if (auto cfg = FindConfiguration(setup.wValue))
        {
            config = cfg;
            remoteWakeupEnabled = !!(config->bmAttributes & ConfigAttributes::RemoteWakeup);
        }
        else
        {
            USBDEBUG("!!! SET_CONFIGURATION called with unknown configuration value %d", setup.wValue);
            async_return(false);
        }
        state = State::Configured;
    }
    else if (state == State::Configured)
    {
        config = NULL;
        state = State::Addressed;
    }

    await(ConfigureEndpoints);
    ControlSuccess();
}
async_end

const ConfigDescriptorHeader* Device::FindConfiguration(uint8_t value)
{
    for (unsigned i = 0; i < configDescriptorCount; i++)
    {
        if (configDescriptors[i]->bConfigurationValue == value)
            return configDescriptors[i];
    }

    return NULL;
}

async(Device::ConfigureEndpoints)
async_def(
    unsigned i;
    const ConfigDescriptorHeader* config;
    const EndpointDescriptor* epConfig;
)
{
    f.config = config;

    if (f.config == NULL)
    {
        // deactivate all endpoints
        for (f.i = 1; f.i <= USB_IN_ENDPOINTS; f.i++)
            await(In(f.i).Configure, NULL);
        for (f.i = 1; f.i <= USB_OUT_ENDPOINTS; f.i++)
            await(Out(f.i).Configure, NULL);
    }
    else
    {
        // configure or reconfigure all endpoints
        for (f.i = 1; f.i <= USB_IN_ENDPOINTS; f.i++)
        {
            f.epConfig = f.config->FindEndpoint(0x80 | f.i);
            await(In(f.i).Configure, f.epConfig);
        }

        for (f.i = 1; f.i <= USB_OUT_ENDPOINTS; f.i++)
        {
            f.epConfig = f.config->FindEndpoint(f.i);
            await(Out(f.i).Configure, f.epConfig);
        }
    }
}
async_end

}
