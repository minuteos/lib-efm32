/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/USART.h
 */

#pragma once

#include <kernel/kernel.h>

#include <base/Span.h>

#include <hw/CMU.h>
#include <hw/GPIO.h>
#include <hw/LDMA.h>

#ifndef USART_TX_TRACE
//! Optional macro for tracing frames transmitted via the USART
#define USART_TX_TRACE(usart, frame)
#endif

#ifndef USART_RX_TRACE
//! Optional macro for tracing frames received via the USART
#define USART_RX_TRACE(usart, frame)
#endif

#ifdef USART0_BASE
#undef USART0
#define USART0	CM_PERIPHERAL(_USART<0>, USART0_BASE)
#endif

#ifdef USART1_BASE
#undef USART1
#define USART1	CM_PERIPHERAL(_USART<1>, USART1_BASE)
#endif

#ifdef USART2_BASE
#undef USART2
#define USART2	CM_PERIPHERAL(_USART<2>, USART2_BASE)
#endif

#ifdef USART3_BASE
#undef USART3
#define USART3	CM_PERIPHERAL(_USART<3>, USART3_BASE)
#endif

#ifdef USART4_BASE
#undef USART4
#define USART4	CM_PERIPHERAL(_USART<4>, USART4_BASE)
#endif

#ifdef USART5_BASE
#undef USART5
#define USART5	CM_PERIPHERAL(_USART<5>, USART5_BASE)
#endif

class USART : public USART_TypeDef
{
public:
    enum
    {
        ClkDivFracBits = 5,                 //< fractional bits in CLKDIV
        ClkDivOne = BIT(ClkDivFracBits),    //< value equal to 1 in CLKDIV
    };

    enum Flags
    {
        ModeAsynchronous = 0,
        ModeSynchronous = USART_CTRL_SYNC,

        Loopback = USART_CTRL_LOOPBK,
        CollisionCheck = USART_CTRL_CCEN,
        MultiProcessorMode = USART_CTRL_MPM,

        MultiProcessorAddress0 = 0,
        MultiProcessorAddress1 = USART_CTRL_MPAB,

        Oversample16 = USART_CTRL_OVS_X16,
        Oversample8 = USART_CTRL_OVS_X8,
        Oversample6 = USART_CTRL_OVS_X6,
        Oversample4 = USART_CTRL_OVS_X4,

        ClockIdleLow = USART_CTRL_CLKPOL_IDLELOW,
        ClockIdleHigh = USART_CTRL_CLKPOL_IDLEHIGH,

        PhaseSampleLeading = USART_CTRL_CLKPHA_SAMPLELEADING,
        PhaseSampleTrailing = USART_CTRL_CLKPHA_SAMPLETRAILING,

        OrderLSBFirst = 0,
        OrderMSBFirst = USART_CTRL_MSBF,

        AutoMasterSlave = USART_CTRL_CSMA,

        TxInterruptLevelEmpty = USART_CTRL_TXBIL_EMPTY,
        TxInterruptLevelHalfFull = USART_CTRL_TXBIL_HALFFULL,

        RxInvert = USART_CTRL_RXINV,
        TxInvert = USART_CTRL_TXINV,
        CSInvert = USART_CTRL_CSINV,

        AutoChipSelect = USART_CTRL_AUTOCS,
        AutoTxTristate = USART_CTRL_AUTOTRI,

        SmartCardMode = USART_CTRL_SCMODE,
        SmartCardRetransmit = USART_CTRL_SCRETRANS,
        SkipParityErrorFrames = USART_CTRL_SKIPPERRF,

        Bit9Default0 = 0,
        Bit9Default1 = USART_CTRL_BIT8DV,

        StopDMAOnError = USART_CTRL_ERRSDMA,
        StopRxOnError = USART_CTRL_ERRSRX,
        StopTxOnError = USART_CTRL_ERRSTX,

        SyncSlaveEarlySetup = USART_CTRL_SSSEARLY,

        Byteswap = USART_CTRL_BYTESWAP,
        AutoTransmit = USART_CTRL_AUTOTX,
        DisableMajorityVote = USART_CTRL_MVDIS,

        SyncMasterDelaySample = USART_CTRL_SMSDELAY,
    };

    enum FlagsEx
    {
        CtsInvert = USART_CTRLX_CTSINV,
        CtsEnable = USART_CTRLX_CTSEN,

        RtsInvert = USART_CTRLX_RTSINV,

        DebugHalt = USART_CTRLX_DBGHALT,

        _Default = _USART_CTRLX_RESETVALUE,
    };

    enum Frame
    {
        FrameBits4 = USART_FRAME_DATABITS_FOUR,
        FrameBits5 = USART_FRAME_DATABITS_FIVE,
        FrameBits6 = USART_FRAME_DATABITS_SIX,
        FrameBits7 = USART_FRAME_DATABITS_SEVEN,
        FrameBits8 = USART_FRAME_DATABITS_EIGHT,
        FrameBits9 = USART_FRAME_DATABITS_NINE,
        FrameBits10 = USART_FRAME_DATABITS_TEN,
        FrameBits11 = USART_FRAME_DATABITS_ELEVEN,
        FrameBits12 = USART_FRAME_DATABITS_TWELVE,
        FrameBits13 = USART_FRAME_DATABITS_THIRTEEN,
        FrameBits14 = USART_FRAME_DATABITS_FOURTEEN,
        FrameBits15 = USART_FRAME_DATABITS_FIFTEEN,
        FrameBits16 = USART_FRAME_DATABITS_SIXTEEN,

        ParityNone = USART_FRAME_PARITY_NONE,
        ParityEven = USART_FRAME_PARITY_EVEN,
        ParityOdd = USART_FRAME_PARITY_ODD,

        StopBitsHalf = USART_FRAME_STOPBITS_HALF,
        StopBitsOne = USART_FRAME_STOPBITS_ONE,
        StopBitsOneAndHalf = USART_FRAME_STOPBITS_ONEANDAHALF,
        StopBitsTwo = USART_FRAME_STOPBITS_TWO,
    };

    //! Gets the index of the USART peripheral
    unsigned Index() const { return ((unsigned)this >> 10) & 0xF; }

    //! Enables the clock to the USART peripheral
    void EnableClock() { CMU->EnableUSART(Index()); }
    //! Disables the clock to the USART peripheral
    void DisableClock() { CMU->DisableUSART(Index()); }
    //! Gets the clock frequency of the USART peripheral
    //! @warning assumes no prescaling in CMU
    unsigned ClockFrequency() const { return _CMU::GetCoreFrequency(); }

    //! Gets the RX IRQ number
    IRQn_Type RxIRQn() const { return (IRQn_Type)BYTES(USART0_RX_IRQn, USART1_RX_IRQn
#if USART_COUNT > 2
        , USART2_RX_IRQn
#endif
#if USART_COUNT > 3
        , USART3_RX_IRQn, USART4_RX_IRQn, USART5_RX_IRQn
#endif
        )[Index()]; }
    //! Gets the TX IRQ number
    IRQn_Type TxIRQn() const { return (IRQn_Type)(RxIRQn() + 1); }

    //! Configures the USART peripheral
    void Setup(Flags flags, FlagsEx flagsEx = FlagsEx::_Default) { CTRL = flags; CTRLX = flagsEx; }
    //! Configures the USART framing
    void FrameSetup(Frame frame) { FRAME = frame; }
    //! Sets the fractional clock divider
    void ClkDiv(uint32_t value) { CLKDIV = ((value - ClkDivOne) << _USART_CLKDIV_DIV_SHIFT) & _USART_CLKDIV_DIV_MASK; }
    //! Gets the fractional clock divider
    unsigned ClkDiv() const { return ((CLKDIV & _USART_CLKDIV_DIV_MASK) >> _USART_CLKDIV_DIV_SHIFT) + ClkDivOne; }

    //! Sets the output clock frequency (clock edge change frequency)
    void OutputClock(unsigned freq) { ClkDiv((ClockFrequency() << ClkDivFracBits) / freq); }
    //! Gets the output clock frequency (clock edge change frequency)
    unsigned OutputClock() const { return (ClockFrequency() << ClkDivFracBits) / ClkDiv(); }

    //! Sets the output frequency in synchronous mode
    void OutputFrequency(unsigned freq) { OutputClock(freq << 1); }
    //! Gets the output frequency in synchronous mode
    unsigned OutputFrequency() { return OutputClock() >> 1; }

    //! Gets the clock oversampling in asynchronous mode
    unsigned ClockOversampling() const { return ClockOversampling(ClockOversamplingIndex()); }

    //! Sets the baud rate in asynchronous mode
    //! @warning assumes 16x oversampling
    void BaudRateUnsafe(unsigned baudRate) { OutputClock(baudRate << 4); }
    //! Gets the baud rate in asynchronous mode
    //! @warning assumes 16x oversampling
    unsigned BaudRateUnsafe() { return OutputClock() >> 4; }

    //! Sets the baud rate and appropriate oversampling in asynchronous mode
    void BaudRate(unsigned baudRate);
    //! Gets the baud rate in asynchronous mode
    unsigned BaudRate() { return OutputClock() / ClockOversampling(); }

    //! Enables data reception
    void RxEnable() { CMD = USART_CMD_RXEN; }
    //! Disables data reception
    void RxDisable() { CMD = USART_CMD_RXDIS; }
    //! Enables data transmission
    void TxEnable() { CMD = USART_CMD_TXEN; }
    //! Disables data transmission
    void TxDisable() { CMD = USART_CMD_TXDIS; }
    //! Enables reception and transmission at the same time
    void BidirectionalEnable() { CMD = USART_CMD_RXEN | USART_CMD_TXEN; }
    //! Disables reception and transmission at the same time
    void BidirectionalDisable() { CMD = USART_CMD_RXDIS | USART_CMD_TXDIS; }
    //! Enables master mode
    void MasterEnable() { CMD = USART_CMD_MASTEREN; }
    //! Disables master mode (enters slave mode)
    void MasterDisable() { CMD = USART_CMD_MASTERDIS; }
    //! Blocks data reception (all received data is discarded)
    void RxBlock() { CMD = USART_CMD_RXBLOCKEN; }
    //! Unblocks data reception
    void RxUnblock() { CMD = USART_CMD_RXBLOCKDIS; }

    //! Enables CTS checking for flow control
    void FlowControlEnable() { EFM32_BITSET_REG(CTRLX, USART_CTRLX_CTSEN); }
    //! Disables CTS checking for flow control
    void FlowControlDisable() { EFM32_BITCLR_REG(CTRLX, USART_CTRLX_CTSEN); }

    //! Checks if data reception is enabled
    bool RxEnabled() { return STATUS & USART_STATUS_RXENS; }
    //! Checks if data reception is blocked (all received data is discarded)
    bool RxBlocked() { return STATUS & USART_STATUS_RXBLOCK; }

    //! Checks if the receive buffer is empty
    bool RxEmpty() { return !(STATUS & USART_STATUS_RXDATAV); }
    //! Checks if there is valid data in the receive buffer
    bool RxValid() { return STATUS & USART_STATUS_RXDATAV; }
    //! Checks if the receive buffer is full
    bool RxFull() { return STATUS & USART_STATUS_RXFULL; }

    //! Checks if data transmission is enabled
    bool TxEnabled() { return STATUS & USART_STATUS_TXENS; }
    //! Checks if the transmit bufer is completely empty
    bool TxEmpty() { return !(STATUS & _USART_STATUS_TXBUFCNT_MASK); }
    //! Gets the number of frames in the transmit buffer
    unsigned TxCount() { return (STATUS & _USART_STATUS_TXBUFCNT_MASK) >> _USART_STATUS_TXBUFCNT_SHIFT; }
    //! Checks if the transmit buffer is full
    bool TxFull() { return !(STATUS & _USART_STATUS_TXBL_MASK); }
    //! Checks if data can be written to the transmit buffer
    bool TxFree() { return STATUS & _USART_STATUS_TXBL_MASK; }
    //! Checks if the transmitter is transmitting data
    bool TxBusy() { return !(STATUS & USART_STATUS_TXIDLE); }
    //! Checks if the transmitter is idle
    bool TxIdle() { return STATUS & USART_STATUS_TXIDLE; }
    //! Checks if the transmitter is tristated
    bool TxTristated() { return STATUS & USART_STATUS_TXTRI; }
    //! Checks if the transmission is completed
    bool TxComplete() { return STATUS & USART_STATUS_TXC; }

    //! Transmits a single frame
    void Transmit(uint32_t data) { TXDATA = data; USART_TX_TRACE(this, data); }
    //! Receives data from a single frame
    uint32_t Receive() { auto res = RXDATA; USART_RX_TRACE(this, res); return res; }

#ifdef EFM32_GPIO_LINEAR_INDEX
    //! Configures the RX pin
    void ConfigureRx(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, ROUTEPEN, 0, 1u); }
    //! Configures the TX pin
    void ConfigureTx(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 1, 0u); }
    //! Configures the CS pin
    void ConfigureCs(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 2, 3u); }
    //! Configures the CLK pin
    void ConfigureClk(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 3, 2u); }
    //! Configures the CTS pin
    void ConfigureCts(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, ROUTEPEN, 4, 4u); }
    //! Configures the RTS pin
    void ConfigureRts(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 5, 5u); }
    //! Gets the location index for the CS pin
    unsigned GetCsLocation(GPIOPin pin) { return pin.GetLinearIndex(3); }
#elif defined(_SILICON_LABS_32B_SERIES_1) && defined(_EFM32_GIANT_FAMILY)
private:
    static const GPIOLocations_t locsRx[], locsTx[], locsCs[], locsClk[], locsCts[], locsRts[];

public:
    //! Configures the RX pin
    void ConfigureRx(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, ROUTEPEN, 0, locsRx[Index()]); }
    //! Configures the TX pin
    void ConfigureTx(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 1, locsTx[Index()]); }
    //! Configures the CS pin
    void ConfigureCs(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 2, locsCs[Index()]); }
    //! Configures the CLK pin
    void ConfigureClk(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 3, locsClk[Index()]); }
    //! Configures the CTS pin
    void ConfigureCts(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, ROUTEPEN, 4, locsCts[Index()]); }
    //! Configures the RTS pin
    void ConfigureRts(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 5, locsRts[Index()]); }
    //! Gets the location index for the CS pin
    unsigned GetCsLocation(GPIOPin pin) { return pin.GetLocation(locsCs[Index()]); }
#elif defined(_SILICON_LABS_32B_SERIES_2)
public:
    //! Configures the RX pin
    void ConfigureRx(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, GPIO_ROUTE_ARGS(USART, RX)); }
    //! Configures the TX pin
    void ConfigureTx(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, GPIO_ROUTE_ARGS(USART, TX)); }
    //! Configures the CS pin
    void ConfigureCs(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, GPIO_ROUTE_ARGS(USART, CS)); }
    //! Configures the CLK pin
    void ConfigureClk(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, GPIO_ROUTE_ARGS(USART, CLK)); }
    //! Configures the CTS pin
    void ConfigureCts(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, GPIO_ROUTE_ARGS_NOPEN(USART, CTS)); }
    //! Configures the RTS pin
    void ConfigureRts(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, GPIO_ROUTE_ARGS(USART, RTS)); }
    //! Gets the location index for the CS pin
    GPIOPinID GetCsLocation(GPIOPin pin) { return pin.GetID(); }
#endif

    //! Attempts to bind to the specified CS pin; returns false if CS is already bound
    bool BindCs(GPIOPin pin) { return BindCs(GetCsLocation(pin)); }
    //! Attempts to bind to the CS pin at the specified location; returns false if CS is already bound
    bool BindCs(unsigned loc);
    //! Attempts to bind to the specified CS pin; returns false if CS is already bound
    async(BindCs, unsigned loc, Timeout timeout = Timeout::Infinite);
#ifdef _SILICON_LABS_32B_SERIES_1
    //! Releases the currently bound CS pin
    void ReleaseCs() { ROUTEPEN &= ~USART_ROUTEPEN_CSPEN; }
#else
    //! Releases the currently bound CS pin
    void ReleaseCs() { GPIO->USARTROUTE_CLR[Index()].ROUTEEN = GPIO_USART_ROUTEEN_CSPEN; }
#endif

    struct SyncTransferDescriptor
    {
        static const uint8_t s_zero;
        static uint8_t s_discard;

        LDMADescriptor rx, tx;

        void Transmit(Span d)
        {
            rx.SetTransfer((const void*)NULL, &s_discard, d.Length(), LDMADescriptor::UnitByte | LDMADescriptor::P2P);
            tx.SetTransfer(d, NULL, d.Length(), LDMADescriptor::UnitByte | LDMADescriptor::M2P);
        }

        void TransmitSame(const uint8_t* src, size_t length)
        {
            rx.SetTransfer((const void*)NULL, &s_discard, length, LDMADescriptor::UnitByte | LDMADescriptor::P2P);
            tx.SetTransfer(src, NULL, length, LDMADescriptor::UnitByte | LDMADescriptor::P2P);
        }

        void Receive(Buffer d)
        {
            rx.SetTransfer((const void*)NULL, d.Pointer(), d.Length(), LDMADescriptor::UnitByte | LDMADescriptor::P2M);
            tx.SetTransfer(&s_zero, NULL, d.Length(), LDMADescriptor::UnitByte | LDMADescriptor::P2P);
        }

        void ReceiveSame(volatile void* dst, size_t length)
        {
            rx.SetTransfer((const void*)NULL, dst, length, LDMADescriptor::UnitByte | LDMADescriptor::P2P);
            tx.SetTransfer(&s_zero, NULL, length, LDMADescriptor::UnitByte | LDMADescriptor::P2P);
        }

        void Bidirectional(Buffer d) { Bidirectional(d, d); }
        void Bidirectional(Span transmit, Buffer receive)
        {
            ASSERT(transmit.Length() == receive.Length());
            rx.SetTransfer((const void*)NULL, receive.Pointer(), receive.Length(), LDMADescriptor::UnitByte | LDMADescriptor::P2M);
            tx.SetTransfer(transmit, NULL, receive.Length(), LDMADescriptor::UnitByte | LDMADescriptor::M2P);
        }

        size_t Length() const { return tx.Count(); }
    };

    //! Starts a simple synchronous bidirectional transfer
    LDMAChannelHandle BeginSyncBidirectionalTransfer(Buffer data);
    //! Performs a simple synchronous bidirectional transfer
    async(SyncBidirectionalTransfer, Buffer data);

    //! Starts a chain of synchronous transfers
    LDMAChannelHandle BeginSyncTransfer(SyncTransferDescriptor* descriptors, size_t count) { return LDMA->GetChannelByIndex(RES_PAIR_FIRST(BeginSyncTransferImpl(descriptors, count))); }
    //! Starts a chain of synchronous transfers
    template<size_t n> ALWAYS_INLINE LDMAChannelHandle BeginSyncTransfer(SyncTransferDescriptor (&descriptors)[n]) { return BeginSyncTransfer(descriptors, n); }

    //! Performs a chain of synchronous transfers
    async(SyncTransfer, SyncTransferDescriptor* descriptors, size_t count);
    //! Performs a chain of synchronous transfers
    template<size_t n> ALWAYS_INLINE async(SyncTransfer, SyncTransferDescriptor (&descriptors)[n]) { return async_forward(SyncTransfer, descriptors, n); }

    //! Performs a single synchronous bidirectional transfer
    async(SyncTransferSingle, uint32_t data);

    //! Sets clock oversampling by index
    void ClockOversamplingIndex(unsigned index) { MODMASK(CTRL, _USART_CTRL_OVS_MASK, index << _USART_CTRL_OVS_SHIFT); }
    //! Gets the currently set clock oversampling index
    unsigned ClockOversamplingIndex() const { return (CTRL & _USART_CTRL_OVS_MASK) >> _USART_CTRL_OVS_SHIFT; }
    //! Gets the actual clock oversampling value by index
    unsigned ClockOversampling(unsigned index) const { return BYTES(16,8,6,4)[index]; }

private:
    RES_PAIR_DECL(BeginSyncTransferImpl, SyncTransferDescriptor* descriptors, size_t count);
};

DEFINE_FLAG_ENUM(USART::Flags);
DEFINE_FLAG_ENUM(USART::FlagsEx);
DEFINE_FLAG_ENUM(USART::Frame);

template<unsigned n> class _USART : public USART
{
#if defined(_SILICON_LABS_32B_SERIES_1) && defined(_EFM32_GIANT_FAMILY)
public:
    //! Configures the RX pin
    void ConfigureRx(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, ROUTEPEN, 0, locRx); }
    //! Configures the TX pin
    void ConfigureTx(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 1, locTx); }
    //! Configures the CS pin
    void ConfigureCs(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 2, locCs); }
    //! Configures the CLK pin
    void ConfigureClk(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 3, locClk); }
    //! Configures the CTS pin
    void ConfifugreCts(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::Input) { pin.ConfigureAlternate(mode, ROUTEPEN, 4, locCts); }
    //! Configures the RTS pin
    void ConfigureRts(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 5, locRts); }

private:
    static const GPIOLocations_t locRx, locTx, locCs, locClk, locCts, locRts;

    friend class USART;
#endif
};
