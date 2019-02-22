/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/USB.h
 */

#pragma once

#include <base/base.h>
#include <base/Span.h>

#ifdef Ckernel
#include <kernel/kernel.h>
#endif

#define USBDEBUG(...)  DBGCL("USB", __VA_ARGS__)

#if USB_DIAGNOSTICS
#define USBDIAG USBDEBUG
#else
#define USBDIAG(...)
#endif

#if USB_ENDPOINT_TRACE
#define USBEPTRACE(index, ptr, len) _CDBG(index, "%b", Span(ptr, len))
#else
#define USBEPTRACE(...)
#endif

#define USB_IN_ENDPOINTS    countof(USB_TypeDef::DIEP)
#define USB_OUT_ENDPOINTS   countof(USB_TypeDef::DOEP)

#define USB_SETUP_PACKET_SIZE   12

#define USB_XFERSIZE_EP0_MAX    _USB_DIEP0TSIZ_XFERSIZE_MASK
#define USB_XFERSIZE_MAX        _USB_DIEP_TSIZ_XFERSIZE_MASK

class USBInEndpoint : public USB_DIEP_TypeDef
{
    enum
    {
        USB_DIEP_CTL_COMMAND_MASK = USB_DIEP_CTL_EPENA |
            USB_DIEP_CTL_EPDIS |
            USB_DIEP_CTL_SETD1PIDOF |
            USB_DIEP_CTL_SETD0PIDEF |
            USB_DIEP_CTL_SNAK |
            USB_DIEP_CTL_CNAK |
            USB_DIEP_CTL_STALL,
    };

public:
    // Gets the index of the endpoint
    unsigned Index() const { return ((uintptr_t)this - (uintptr_t)(&USB->DIEP0CTL)) / sizeof(USBInEndpoint); }
    class _USB* Owner() const { return CM_PERIPHERAL(_USB, USB_BASE); } // there is just one USB peripheral

    // Gets the maximum packet size for the endpoint
    unsigned PacketSize() const
    {
        if (Index() == 0)
            return 64 >> ((CTL & _USB_DIEP0CTL_MPS_MASK) >> _USB_DIEP0CTL_MPS_SHIFT);
        else
            return (CTL & _USB_DIEP_CTL_MPS_MASK) >> _USB_DIEP_CTL_MPS_SHIFT;
    }

    // Gets the maximum transfer size for the endpoint
    unsigned MaxTransferSize() const { return Index() == 0 ? USB_XFERSIZE_EP0_MAX : USB_XFERSIZE_MAX; }

    //! Determines if the endpoint has data ready to be transmitted
    bool IsEnabled() { return CTL & USB_DIEP_CTL_EPENA; }
    //! Determines if the endpoint is responding to requests
    bool IsActive() { return CTL & USB_DIEP_CTL_USBACTEP; }
    //! Determines if the endpoint is stalled
    bool IsStalled() { return CTL & USB_DIEP_CTL_STALL; }

    //! Abort current transaction and set NAK
    void Reset() { Control((IsEnabled() * USB_DIEP_CTL_EPDIS) | USB_DIEP_CTL_SNAK); }
    //! Make the endpoint unresponsive
    void Disable() { Control((IsEnabled() * USB_DIEP_CTL_EPDIS) | USB_DIEP_CTL_CNAK); }
    //! Stall the endpoint
    void Stall() { Control((IsEnabled() * USB_DIEP_CTL_EPDIS) | USB_DIEP_CTL_STALL); }
    //! Unstall the endpoint
    void Unstall() { Control(USB_DIEP_CTL_SETD0PIDEF); }
    //! Stall or unstall the endpoint
    void Stall(bool stall) { if (stall != IsStalled()) { stall ? Stall() : Unstall(); } }

    //! Activate the endpoint with the specified packet size and type
    void Activate(size_t maxPacketSize, unsigned type, unsigned fifo)
    {
        CTL = (CTL & ~(USB_DIEP_CTL_COMMAND_MASK | _USB_DIEP_CTL_EPTYPE_MASK | _USB_DIEP_CTL_TXFNUM_MASK | _USB_DIEP_CTL_MPS_MASK)) |
            (maxPacketSize << _USB_DIEP_CTL_MPS_SHIFT) |
            (type << _USB_DIEP_CTL_EPTYPE_SHIFT) |
            (fifo << _USB_DIEP_CTL_TXFNUM_SHIFT) |
            USB_DIEP_CTL_USBACTEP |
            USB_DIEP_CTL_SNAK;
    }

    //! Deactiate the endpoint
    void Deactivate() { CTL &= ~(USB_DIEP_CTL_COMMAND_MASK | USB_DIEP_CTL_USBACTEP); }

    //! Reads and clears current interrupt state
    uint32_t InterruptClear() { auto state = INT; INT = state; return state; }
    //! Enables the interrupt for the endpoint
    void InterruptEnable();
    //! Disables the interrupt for the endpoint
    void InterruptDisable();

    void TransmitPacket(Span buffer)
    {
        if (buffer.Length())
        {
            ASSERT(buffer.Length() <= MaxTransferSize());

            auto maxpkt = PacketSize();
            TSIZ = (((buffer.Length() - 1) / maxpkt + 1) << _USB_DIEP_TSIZ_PKTCNT_SHIFT) |
                (buffer.Length() << _USB_DIEP_TSIZ_XFERSIZE_SHIFT);
            DMAADDR = (uint32_t)buffer.Pointer();
        }
        else
        {
            // zero-length packet
            TSIZ = 1 << _USB_DIEP_TSIZ_PKTCNT_SHIFT;
        }
        Control(USB_DIEP_CTL_EPENA | USB_DIEP_CTL_CNAK);
    }

    void Control(uint32_t cmd) { CTL = (CTL & ~USB_DIEP_CTL_COMMAND_MASK) | cmd; }
};

class USBOutEndpoint : public USB_DOEP_TypeDef
{
    enum
    {
        USB_DOEP_CTL_COMMAND_MASK = USB_DOEP_CTL_EPENA |
            USB_DOEP_CTL_EPDIS |
            USB_DOEP_CTL_SETD1PIDOF |
            USB_DOEP_CTL_SETD0PIDEF |
            USB_DOEP_CTL_SNAK |
            USB_DOEP_CTL_CNAK |
            USB_DOEP_CTL_STALL,
    };

public:
    // Gets the index on the endpoint
    unsigned Index() const { return ((uintptr_t)this - (uintptr_t)(&USB->DOEP0CTL)) / sizeof(USBInEndpoint); }

    // Gets the maximum packet size for the endpoint
    unsigned PacketSize() const
    {
        if (Index() == 0)
            return 64 >> ((CTL & _USB_DOEP0CTL_MPS_MASK) >> _USB_DOEP0CTL_MPS_SHIFT);
        else
            return (CTL & _USB_DOEP_CTL_MPS_MASK) >> _USB_DOEP_CTL_MPS_SHIFT;
    }

    // Gets the maximum transfer size for the endpoint
    unsigned MaxTransferSize() const { return Index() == 0 ? USB_XFERSIZE_EP0_MAX : USB_XFERSIZE_MAX; }

    //! Retrieves the number of setup packets before running out of buffer space
    unsigned SetupCount() { return (TSIZ & _USB_DOEP0TSIZ_SUPCNT_MASK) >> _USB_DOEP0TSIZ_SUPCNT_SHIFT; }

    //! Pointer to where DMA will write received data
    void* Pointer() { return (void*)DMAADDR; }

    //! Determines if the endpoint is ready to receive data
    bool IsEnabled() { return CTL & USB_DOEP_CTL_EPENA; }
    //! Determines if the endpoint is responding to requests
    bool IsActive() { return CTL & USB_DOEP_CTL_USBACTEP; }
    //! Determines if the endpoint is stalled
    bool IsStalled() { return CTL & USB_DOEP_CTL_STALL; }

    //! Abort current transaction and set NAK
    void Reset() { Control((IsEnabled() * USB_DOEP_CTL_EPDIS) | USB_DOEP_CTL_SNAK); }
    //! Make the endpoint unresponsive
    void Disable() { Control((IsEnabled() * USB_DOEP_CTL_EPDIS) | USB_DOEP_CTL_CNAK); }
    //! Stall the endpoint
    void Stall() { Control((IsEnabled() * USB_DOEP_CTL_EPDIS) | USB_DOEP_CTL_STALL); }
    //! Unstall the endpoint
    void Unstall() { Control(USB_DIEP_CTL_SETD0PIDEF); }
    //! Stall or unstall the endpoint
    void Stall(bool stall) { if (stall != IsStalled()) { stall ? Stall() : Unstall(); } }

    //! Activate the endpoint with the specified packet size and type
    void Activate(size_t maxPacketSize, unsigned type) { Control(USB_DOEP_CTL_USBACTEP | (maxPacketSize << _USB_DOEP_CTL_MPS_SHIFT) | (type << _USB_DOEP_CTL_EPTYPE_SHIFT) | USB_DOEP_CTL_SNAK); }
    //! Deactiate the endpoint
    void Deactivate() { CTL &= ~(USB_DOEP_CTL_COMMAND_MASK | USB_DOEP_CTL_USBACTEP); }

    //! Reads and clears current interrupt state
    uint32_t InterruptClear() { auto state = INT; INT = state; return state; }
    //! Enables the interrupt for the endpoint
    void InterruptEnable();
    //! Disables the interrupt for the endpoint
    void InterruptDisable();

    //! Configures the endpoint to receive SETUP request packets
    void ConfigureSetup(void* buffer, size_t count)
    {
        ASSERT((uintptr_t)this == (uintptr_t)&(USB->DOEP0CTL));
        ASSERT(count <= 3);

        TSIZ = (count << _USB_DOEP0TSIZ_SUPCNT_SHIFT) |
            (1 << _USB_DOEP0TSIZ_PKTCNT_SHIFT) |
            ((count * USB_SETUP_PACKET_SIZE) << _USB_DOEP0TSIZ_XFERSIZE_SHIFT);
        DMAADDR = (uint32_t)buffer;
        Control(USB_DOEP_CTL_EPENA);    // enable but keep NAK set
    }

    //! Configures the endpoint to receive a packet
    void ReceivePacket(Buffer buffer)
    {
        if (buffer.Length())
        {
            ASSERT(buffer.Length() <= MaxTransferSize());

            auto maxpkt = PacketSize();
            TSIZ = (((buffer.Length() - 1) / maxpkt + 1) << _USB_DIEP_TSIZ_PKTCNT_SHIFT) |
                (buffer.Length() << _USB_DIEP_TSIZ_XFERSIZE_SHIFT);
            DMAADDR = (uint32_t)buffer.Pointer();
        }
        else
        {
            // zero-length packet
            TSIZ = 1 << _USB_DIEP_TSIZ_PKTCNT_SHIFT;
        }
        Control(USB_DIEP_CTL_EPENA | USB_DIEP_CTL_CNAK);
    }

    //! Gets the length of the last received packet
    uint32_t ReceivedLength() { return TSIZ & _USB_DIEP_TSIZ_XFERSIZE_MASK; }

private:
    void Control(uint32_t cmd) { CTL = (CTL & ~USB_DOEP_CTL_COMMAND_MASK) | cmd; }
};

#undef USB
#define USB CM_PERIPHERAL(_USB, USB_BASE)

class _USB : public USB_TypeDef
{
public:
    enum DeviceFlags
    {
        SpeedFull = USB_DCFG_DEVSPD_FS,
        SpeedLow = USB_DCFG_DEVSPD_LS,

        PeriodicFrame80 = USB_DCFG_PERFRINT_80PCNT,
        PeriodicFrame85 = USB_DCFG_PERFRINT_85PCNT,
        PeriodicFrame90 = USB_DCFG_PERFRINT_90PCNT,
        PeriodicFrame95 = USB_DCFG_PERFRINT_95PCNT,

        NonZeroLengthStatusOutHandshake = USB_DCFG_NZSTSOUTHSHK,

        _SetupMask = _USB_DCFG_DEVSPD_MASK | _USB_DCFG_PERFRINT_MASK | USB_DCFG_NZSTSOUTHSHK,
    };

    enum ThresholdFlags
    {
        ThresholdNone = 0,
        ThresholdRx = USB_DTHRCTL_RXTHREN,
        ThresholdIsoTx = USB_DTHRCTL_ISOTHREN,
        ThresholdNonIsoTx = USB_DTHRCTL_NONISOTHREN,
        ThresholdTx = ThresholdIsoTx | ThresholdNonIsoTx,

        _ThresholdMask = _USB_DTHRCTL_RXTHREN_MASK | _USB_DTHRCTL_ISOTHREN_MASK | _USB_DTHRCTL_NONISOTHREN_DEFAULT,
    };

    void EnablePHYPins() { ROUTE |= USB_ROUTE_PHYPEN; }
    void EnableVBUSENPin() { ROUTE |= USB_ROUTE_VBUSENPEN; }

    void ForceDevice() { MODMASK(GUSBCFG, USB_GUSBCFG_FORCEDEVMODE | USB_GUSBCFG_FORCEHSTMODE, USB_GUSBCFG_FORCEDEVMODE); }
    void ForceHost() { MODMASK(GUSBCFG, USB_GUSBCFG_FORCEDEVMODE | USB_GUSBCFG_FORCEHSTMODE, USB_GUSBCFG_FORCEHSTMODE); }

    void DeviceSetup(DeviceFlags flags) { MODMASK_SAFE(DCFG, _SetupMask, flags); }
    void DeviceAddress(uint32_t addr) { MODMASK_SAFE(DCFG, _USB_DCFG_DEVADDR_MASK, addr << _USB_DCFG_DEVADDR_SHIFT); }

    void DeviceConnect() { DCTL &= ~USB_DCTL_SFTDISCON; }
    void DeviceDisconnect() { DCTL |= USB_DCTL_SFTDISCON; }
    void DeviceControl(uint32_t ctl) { DCTL |= ctl; }
    void DeviceGlobalInNak(bool set) { DeviceControl(set ? USB_DCTL_SGNPINNAK : USB_DCTL_CGNPINNAK); }
    void DeviceGlobalOutNak(bool set) { DeviceControl(set ? USB_DCTL_SGOUTNAK : USB_DCTL_CGOUTNAK); }

    bool DeviceFullSpeed() { return (DSTS & _USB_DSTS_ENUMSPD_MASK) == USB_DSTS_ENUMSPD_FS; }

    uint32_t DeviceEndpointInterrupts() { return DAINT & DAINTMSK; }

    void DeviceThresholdEnable(ThresholdFlags flags) { MODMASK_SAFE(DTHRCTL, _ThresholdMask, flags); }

    void RxFifoSetup(size_t words) { GRXFSIZ = words << _USB_GRXFSIZ_RXFDEP_SHIFT; }
    void TxFifo0Setup(size_t start, size_t words) { GNPTXFSIZ = (start << _USB_GNPTXFSIZ_NPTXFSTADDR_SHIFT) | (words << _USB_GNPTXFSIZ_NPTXFINEPTXF0DEP_SHIFT); }
    void TxFifoSetup(unsigned n, size_t start, size_t words) { (&DIEPTXF1)[n - 1] = (start << _USB_DIEPTXF1_INEPNTXFSTADDR_SHIFT) | (words << _USB_DIEPTXF1_INEPNTXFDEP_SHIFT); }

    void TxFifoFlush(unsigned n) { GRSTCTL = USB_GRSTCTL_TXFFLSH | (n << _USB_GRSTCTL_TXFFLSH_SHIFT); while (GRSTCTL & USB_GRSTCTL_TXFFLSH); }
    void TxFifoFlushAll() { GRSTCTL = USB_GRSTCTL_TXFFLSH | USB_GRSTCTL_TXFNUM_FALL; while (GRSTCTL & USB_GRSTCTL_TXFFLSH); }
    void RxFifoFlush() { GRSTCTL = USB_GRSTCTL_RXFFLSH; while (GRXSTSP & USB_GRSTCTL_RXFFLSH); }

    USBInEndpoint& In(unsigned n) { return ((USBInEndpoint*)&DIEP0CTL)[n]; }
    USBOutEndpoint& Out(unsigned n) { return ((USBOutEndpoint*)&DOEP0CTL)[n]; }

    void IRQEnable() { NVIC_EnableIRQ(USB_IRQn); }
    void IRQDisable() { NVIC_EnableIRQ(USB_IRQn); }
    void IRQClear() { NVIC_ClearPendingIRQ(USB_IRQn); }
    void IRQHandler(Delegate<void> handler) { Cortex_SetIRQHandler(USB_IRQn, handler); }

    void CoreInterruptEnable() { GAHBCFG |= USB_GAHBCFG_GLBLINTRMSK; }
    void CoreInterruptDisable() { GAHBCFG &= ~USB_GAHBCFG_GLBLINTRMSK; }

#ifdef Ckernel
    async(CoreEnable);
    async(CoreReset);
#endif
};

DEFINE_FLAG_ENUM(_USB::DeviceFlags);
DEFINE_FLAG_ENUM(_USB::ThresholdFlags);

ALWAYS_INLINE void USBInEndpoint::InterruptEnable() { USB->DAINTMSK |= USB_DAINTMSK_INEPMSK0 << Index(); }
ALWAYS_INLINE void USBInEndpoint::InterruptDisable() { USB->DAINTMSK &= ~(USB_DAINTMSK_INEPMSK0 << Index()); }
ALWAYS_INLINE void USBOutEndpoint::InterruptEnable() { USB->DAINTMSK |= USB_DAINTMSK_OUTEPMSK0 << Index(); }
ALWAYS_INLINE void USBOutEndpoint::InterruptDisable() { USB->DAINTMSK |= ~(USB_DAINTMSK_OUTEPMSK0 << Index()); }
