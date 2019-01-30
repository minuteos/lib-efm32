/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-usb/usb/Device.h
 */

#pragma once

#include <kernel/kernel.h>

#include <hw/USB.h>

#include <usb/Packets.h>
#include <usb/Descriptors.h>
#include <usb/DeviceEndpoints.h>

namespace usb
{

typedef Delegate<void, SetupPacket, Span> ControlDelegate;

class Device
{
public:
    template<typename TStringTable, typename... TConfig> Device(const DeviceDescriptor& deviceDescriptor, 
        const TStringTable& strings,
        const TConfig&... configs) :
        deviceDescriptor(deviceDescriptor),
        stringDescriptors(strings),
        configDescriptorCount(sizeof...(TConfig))
    {
        static const ConfigDescriptorHeader* configArray[] = { &configs... };
        configDescriptors = configArray;
    }

    void Start(ControlDelegate controlCallback = ControlDelegate())
    {
        ctrl.callback = controlCallback;
        kernel::Task::Run(this, &Device::Task);
    }

    void ControlSuccess(Span span = Span());

    DeviceInEndpoint& In(unsigned n) { ASSERT(n > 0 && n <= USB_IN_ENDPOINTS); return in[n - 1]; }
    DeviceOutEndpoint& Out(unsigned n) { ASSERT(n > 0 && n <= USB_OUT_ENDPOINTS); return out[n - 1]; }

private:
    enum struct State : uint8_t
    {
        None, Default, Addressed, Configured,
    };

    enum struct ControlState : uint8_t
    {
        Idle,
        DataRx,
        Processing,
        DataTx,
        StatusRx,
        StatusTx,
        Stall,
    };

    enum struct Tasks : uint8_t
    {
        None = 0,
        Control = 1,
    };

    DECLARE_FLAG_ENUM(Tasks);

    enum
    {
        USB_GINTMSK_INIT = USB_GINTMSK_USBRSTMSK | USB_GINTMSK_ENUMDONEMSK | USB_GINTMSK_USBSUSPMSK,
        USB_GINTMSK_ALL = USB_GINTMSK_INIT | USB_GINTMSK_IEPINTMSK | USB_GINTMSK_OEPINTMSK | USB_GINTMSK_WKUPINTMSK,
    };

    _USB* usb = USB;
    const ConfigDescriptorHeader* config = NULL;
    State state = State::None;
    Tasks tasks = Tasks::None;

    struct
    {
        ControlDelegate callback;        
        Span txRemain;
        SetupPacket setup;
        union
        {
            SetupPacket setupBuffer[3];
            uint8_t data[64];
            uint16_t data16;
            uint32_t data32;
        };
        ControlState state = ControlState::Idle;
        bool hasResult;
    } ctrl;
    bool suspended = false;
    bool remoteWakeupEnabled = false;
    bool fullSpeed;
    const DeviceDescriptor& deviceDescriptor;
    const ConfigDescriptorHeader** configDescriptors;
    const StringDescriptor* stringDescriptors;
    size_t configDescriptorCount;
    DeviceInEndpoint in[USB_IN_ENDPOINTS];
    DeviceOutEndpoint out[USB_IN_ENDPOINTS];

    async(Task);
    void IRQHandler();

    void EnableInitInterrupts() { usb->GINTMSK = USB_GINTMSK_INIT; }
    void EnableAllInterrupts() { usb->GINTMSK = USB_GINTMSK_ALL; }
    void ControlSetup() { usb->Out(0).ConfigureSetup(ctrl.setupBuffer, 3); ctrl.state = ControlState::Idle; }
    void ControlStall() { usb->Out(0).Stall(); usb->In(0).Stall(); ControlSetup(); ctrl.state = ControlState::Stall; }

    void AllocateFifo();
    void HandleOut();
    void HandleOutControl();
    void HandleIn();
    void HandleInControl();
    
    async(HandleControl);
    void HandleControlStandard(SetupPacket setup);
    void HandleControlGetStatus(SetupPacket setup);
    void HandleControlFeature(SetupPacket setup, bool set);
    void HandleControlSetAddress(SetupPacket setup);
    void HandleControlGetDescriptor(SetupPacket setup);
    void HandleControlGetConfiguration(SetupPacket setup);
    async(HandleControlSetConfiguration, SetupPacket setup);

    const ConfigDescriptorHeader* FindConfiguration(uint8_t value);
    
    async(ConfigureEndpoints);
    async(ConfigureEndpoint, DeviceInEndpoint& ep, const EndpointDescriptor* cfg);
    async(ConfigureEndpoint, DeviceOutEndpoint& ep, const EndpointDescriptor* cfg);
    void ReconfigureInterface(int interface, int altConfig);
};

DEFINE_FLAG_ENUM(Device::Tasks);

}
