/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/LDMA.h
 */

#pragma once

#include <kernel/kernel.h>

#include <hw/CMU.h>

#undef LDMA
#define LDMA	CM_PERIPHERAL(LDMAController, LDMA_BASE)

//! Smart type for specifying link values for the LINK field in LDMA controller
class LDMALink
{
public:
    enum FixedValue
    {
        None = 0,
        Self = 3,
        Next = 0x13,
        Previous = 0xFFFFFFF3,
    };

    //! Creates an LDMA LINK from an absolute pointer
    constexpr LDMALink(class LDMADescriptor* ptr) : value(ptr ? (uint32_t)ptr | 2 : 0) {}
    //! Creates an LDMA LINK from one of the fixed values
    constexpr LDMALink(LDMALink::FixedValue value) : value((uint32_t)value) {}

private:
    uint32_t value;

    static constexpr LDMADescriptor* Decode(uint32_t value, const volatile void* relativeTo)
    {
        return value & 2 ?
            (LDMADescriptor*)(((value & 1) ? (intptr_t)relativeTo : 0) + (value & ~3)) :
            NULL;
    }

    friend class LDMADescriptor;
};

//! LDMA transfer descriptor
//! structure matches the four words starting with LDMA_CH_TypeDef::CTRL exactly
class LDMADescriptor
{
public:
    uint32_t CTRL;         /**< Channel Descriptor Control Word Register  */
    uint32_t SRC;          /**< Channel Descriptor Source Data Address Register  */
    uint32_t DST;          /**< Channel Descriptor Destination Data Address Register  */
    uint32_t LINK;         /**< Channel Descriptor Link Structure Address Register  */

    enum
    {
        TypeTransfer = LDMA_CH_CTRL_STRUCTTYPE_TRANSFER,
        TypeSync = LDMA_CH_CTRL_STRUCTTYPE_SYNCHRONIZE,
        TypeImmediate = LDMA_CH_CTRL_STRUCTTYPE_WRITE,

        RelativeSource = LDMA_CH_CTRL_SRCMODE_RELATIVE,
        RelativeDestination = LDMA_CH_CTRL_DSTMODE_RELATIVE,

        MaximumTransferSize = (_LDMA_CH_CTRL_XFERCNT_MASK >> _LDMA_CH_CTRL_XFERCNT_SHIFT) + 1,
    };

public:
    enum Flags
    {
        Start = LDMA_CH_CTRL_STRUCTREQ,

        ByteSwap = LDMA_CH_CTRL_BYTESWAP,

        BlockSize1 = LDMA_CH_CTRL_BLOCKSIZE_UNIT1,
        BlockSize2 = LDMA_CH_CTRL_BLOCKSIZE_UNIT2,
        BlockSize3 = LDMA_CH_CTRL_BLOCKSIZE_UNIT3,
        BlockSize4 = LDMA_CH_CTRL_BLOCKSIZE_UNIT4,
        BlockSize6 = LDMA_CH_CTRL_BLOCKSIZE_UNIT6,
        BlockSize8 = LDMA_CH_CTRL_BLOCKSIZE_UNIT8,
        BlockSize16 = LDMA_CH_CTRL_BLOCKSIZE_UNIT16,
        BlockSize32 = LDMA_CH_CTRL_BLOCKSIZE_UNIT32,
        BlockSize64 = LDMA_CH_CTRL_BLOCKSIZE_UNIT64,
        BlockSize128 = LDMA_CH_CTRL_BLOCKSIZE_UNIT128,
        BlockSize256 = LDMA_CH_CTRL_BLOCKSIZE_UNIT256,
        BlockSize512 = LDMA_CH_CTRL_BLOCKSIZE_UNIT512,
        BlockSize1024 = LDMA_CH_CTRL_BLOCKSIZE_UNIT1024,
        BlockSizeAll = LDMA_CH_CTRL_BLOCKSIZE_ALL,

#ifdef LDMA_CH_CTRL_DONEIFSEN
        SetDone = LDMA_CH_CTRL_DONEIFSEN,
#else
        SetDone = LDMA_CH_CTRL_DONEIEN,
#endif

        TransferModeBlock = LDMA_CH_CTRL_REQMODE_BLOCK,
        TransferModeAll = LDMA_CH_CTRL_REQMODE_ALL,

        DecrementLoop = LDMA_CH_CTRL_DECLOOPCNT,

        IgnoreSingleRequests = LDMA_CH_CTRL_IGNORESREQ,

        SourceIncrement1 = LDMA_CH_CTRL_SRCINC_ONE,
        SourceIncrement2 = LDMA_CH_CTRL_SRCINC_TWO,
        SourceIncrement4 = LDMA_CH_CTRL_SRCINC_FOUR,
        SourceIncrement0 = LDMA_CH_CTRL_SRCINC_NONE,

        UnitByte = LDMA_CH_CTRL_SIZE_BYTE,
        UnitHalfWord = LDMA_CH_CTRL_SIZE_HALFWORD,
        UnitWord = LDMA_CH_CTRL_SIZE_WORD,

        DestinationIncrement1 = LDMA_CH_CTRL_DSTINC_ONE,
        DestinationIncrement2 = LDMA_CH_CTRL_DSTINC_TWO,
        DestinationIncrement4 = LDMA_CH_CTRL_DSTINC_FOUR,
        DestinationIncrement0 = LDMA_CH_CTRL_DSTINC_NONE,

        // some common flag combinations follow
        M2M = SourceIncrement1 | DestinationIncrement1,
        M2P = SourceIncrement1 | DestinationIncrement0,
        P2M = SourceIncrement0 | DestinationIncrement1,
        P2P = SourceIncrement0 | DestinationIncrement0,
    };

    //! Clears the descriptor
    void Reset()
    {
        SRC = DST = LINK = CTRL = 0;
    }

    //! Configures the descriptor to transfer data between two absolute memory locations
    void SetTransfer(volatile const void* source, volatile void* destination, size_t count, Flags flags, LDMALink link = LDMALink::None)
    {
        *this = Transfer(source, destination, count, flags, link);
    }

    //! Creates a descriptor to transfer data between two absolute memory locations
    static constexpr LDMADescriptor Transfer(volatile const void* source, volatile void* destination, size_t count, Flags flags, LDMALink link = LDMALink::None)
    {
        ASSERT(!(((count - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT) & ~_LDMA_CH_CTRL_XFERCNT_MASK));
        return {
            TypeTransfer | flags | ((count - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT),
            (uint32_t)source,
            (uint32_t)destination,
            link.value
        };
    }

    //! Configures the descriptor to transfer data from a location relative to descriptor address to an abosolute memory location
    void SetTransfer(ptrdiff_t relativeSource, volatile void* destination, size_t count, Flags flags, LDMALink link = LDMALink::None)
    {
        *this = Transfer(relativeSource, destination, count, flags, link);
    }

    //! Creates a descriptor to transfer data from a location relative to descriptor address to an abosolute memory location
    static constexpr LDMADescriptor Transfer(ptrdiff_t relativeSource, volatile void* destination, size_t count, Flags flags, LDMALink link = LDMALink::None)
    {
        ASSERT(!(((count - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT) & ~_LDMA_CH_CTRL_XFERCNT_MASK));
        return {
            TypeTransfer | RelativeSource | flags | ((count - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT),
            (uint32_t)relativeSource,
            (uint32_t)destination,
            link.value
        };
    }

    //! Configures the descriptor to write a value to an absolute memory location
    void SetImmediateWrite(uint32_t data, volatile void* destination, LDMALink link = LDMALink::None)
    {
        *this = ImmediateWrite(data, destination, link);
    }

    //! Creates a descriptor to write a value to an absolute memory location
    static constexpr LDMADescriptor ImmediateWrite(uint32_t data, volatile void* destination, LDMALink link = LDMALink::None)
    {
        return {
            TypeImmediate,
            data,
            (uint32_t)destination,
            link.value
        };
    }

    //! Checks if the descriptor specified a memory transfer operation
    ALWAYS_INLINE bool IsTransfer() volatile const { return (CTRL & _LDMA_CH_CTRL_STRUCTTYPE_MASK) == TypeTransfer; }
    //! Checks if the descriptor specified an immediate write operation
    ALWAYS_INLINE bool IsImmediate() volatile const { return (CTRL & _LDMA_CH_CTRL_STRUCTTYPE_MASK) == TypeImmediate; }
    //! Checks if the descriptor specified a synchronization operation
    ALWAYS_INLINE bool IsSync() volatile const { return (CTRL & _LDMA_CH_CTRL_STRUCTTYPE_MASK) == TypeSync; }

    //! Gets the source address of the transfer
    ALWAYS_INLINE const void* Source() volatile const { return (const void*)SRC; }
    //! Sets the source address of the transfer
    ALWAYS_INLINE void Source(const volatile void* ptr) volatile { SRC = (uint32_t)ptr; }
    //! Gets the destination address of the transfer
    ALWAYS_INLINE void* Destination() volatile const { return (void*)DST; }
    //! Sets the destination address of the transfer
    ALWAYS_INLINE void Destination(volatile void* ptr) volatile { DST = (uint32_t)ptr; }
    //! Gets the number of units transfered (depends on Flags)
    ALWAYS_INLINE size_t Count() volatile const { return ((CTRL & _LDMA_CH_CTRL_XFERCNT_MASK) >> _LDMA_CH_CTRL_XFERCNT_SHIFT) + 1; }
    //! Sets the number of units transfered (depends on Flags)
    ALWAYS_INLINE void Count(size_t val) volatile { MODMASK_SAFE(CTRL, _LDMA_CH_CTRL_XFERCNT_MASK, (val - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT); }
    //! Sets the linked descriptor to be used after this one completes
    ALWAYS_INLINE void Link(LDMALink link) volatile { LINK = link.value; }
    //! Gets the descriptor this one links to
    ALWAYS_INLINE LDMADescriptor* LinkedDescriptor() volatile const { return LDMALink::Decode(LINK, this); }
    //! Sets the done interrupt flag after the descriptor completes
    ALWAYS_INLINE void DoneInterrupt(bool set = true) volatile { CTRL |= Flags::SetDone; }

    //! Gets the LDMAChannel to which this descriptor belongs
    //! @warning Can be used only on the primary descriptor obtained from an LDMAChannel
    ALWAYS_INLINE class LDMAChannel& Channel() volatile const { class LDMAChannel* ch = (LDMAChannel*)((uintptr_t)this - offsetof(LDMA_CH_TypeDef, CTRL)); return *ch; }
    //! Gets the LDMAChannelHandle of the channel to which this descriptor belongs
    //! @warning Can be used only on the primary descriptor obtained from an LDMAChannel
    class LDMAChannelHandle ChannelHandle() volatile const;

    friend class LDMAChannelHandle;
};

DEFINE_FLAG_ENUM(LDMADescriptor::Flags);

class LDMAChannel : public LDMA_CH_TypeDef
{
    friend class LDMAController;
    friend class LDMAChannelHandle;

public:
    //! Gets the primary LDMADescriptor of this channel
    LDMADescriptor& Descriptor() { return *(LDMADescriptor*)&CTRL; }

private:
#ifdef LDMAXBAR
#define LDMA_SOURCESEL(src) _LDMAXBAR_CH_REQSEL_SOURCESEL_ ## src
#define LDMA_SIGSEL(src)    _LDMAXBAR_CH_REQSEL_SIGSEL_ ## src
#else
#define LDMA_SOURCESEL(src) _LDMA_CH_REQSEL_SOURCESEL_ ## src
#define LDMA_SIGSEL(src)    _LDMA_CH_REQSEL_SIGSEL_ ## src
#endif

    enum Source
    {
        SourceNone =    LDMA_SOURCESEL(NONE),
#ifdef _LDMA_CH_REQSEL_SOURCESEL_PRS
        SourcePRS =     LDMA_SOURCESEL(PRS),
#else
        SourcePRS =     LDMA_SOURCESEL(LDMAXBAR),
#endif
#if ADC_COUNT
        SourceADC0 =    LDMA_SOURCESEL(ADC0),
#endif
#if ADC_COUNT > 1
        SourceADC1 =    LDMA_SOURCESEL(ADC0),
#endif
#if IADC_COUNT
        SourceIADC0 =   LDMA_SOURCESEL(IADC0),
#endif
#if VDAC_COUNT
        SourceVDAC0 =   LDMA_SOURCESEL(VDAC0),
#endif
        SourceUSART0 =  LDMA_SOURCESEL(USART0),
        SourceUSART1 =  LDMA_SOURCESEL(USART1),
#if USART_COUNT > 2
        SourceUSART2 =  LDMA_SOURCESEL(USART2),
#endif
#if USART_COUNT > 3
        SourceUSART3 =  LDMA_SOURCESEL(USART3),
#endif
#if USART_COUNT > 4
        SourceUSART4 =  LDMA_SOURCESEL(USART4),
#endif
#if USART_COUNT > 5
        SourceUSART5 =  LDMA_SOURCESEL(USART5),
#endif
#if UART_COUNT
        SourceUART0 =   LDMA_SOURCESEL(UART0),
        SourceUART1 =   LDMA_SOURCESEL(UART1),
#endif
#if LEUART_COUNT
        SourceLEUART0 = LDMA_SOURCESEL(LEUART0),
#endif
#if LEUART_COUNT > 1
        SourceLEUART1 = LDMA_SOURCESEL(LEUART1),
#endif
        SourceI2C0 =    LDMA_SOURCESEL(I2C0),
#if IDC_COUNT > 1
        SourceI2C1 =    LDMA_SOURCESEL(I2C1),
#endif
#if IDC_COUNT > 2
        SourceI2C2 =    LDMA_SOURCESEL(I2C2),
#endif
        SourceTIMER0 =  LDMA_SOURCESEL(TIMER0),
        SourceTIMER1 =  LDMA_SOURCESEL(TIMER1),
#if TIMER_COUNT > 2
        SourceTIMER2 =  LDMA_SOURCESEL(TIMER2),
#endif
#if TIMER_COUNT > 3
        SourceTIMER3 =  LDMA_SOURCESEL(TIMER3),
#endif
#if TIMER_COUNT > 4
        SourceTIMER4 =  LDMA_SOURCESEL(TIMER4),
#endif
#if TIMER_COUNT > 5
        SourceTIMER5 =  LDMA_SOURCESEL(TIMER5),
#endif
#if TIMER_COUNT > 6
        SourceTIMER6 =  LDMA_SOURCESEL(TIMER6),
#endif
        SourceMSC    =  LDMA_SOURCESEL(MSC),
#ifdef _LDMA_CH_REQSEL_SOURCESEL_CRYPTO0
        SourceCRYPTO =  LDMA_SOURCESEL(CRYPTO0),
#elif defined(_LDMA_CH_REQSEL_SOURCESEL_CRYPTO)
        SourceCRYPTO =  LDMA_SOURCESEL(CRYPTO),
#endif
#if PDM_COUNT
        SourcePDM0   =  LDMA_SOURCESEL(PDM),
#endif
    };

public:
    enum Flags
    {
        ArbitrationSlots1 = LDMA_CH_CFG_ARBSLOTS_ONE,
        ArbitrationSlots2 = LDMA_CH_CFG_ARBSLOTS_TWO,
        ArbitrationSlots4 = LDMA_CH_CFG_ARBSLOTS_FOUR,
        ArbitrationSlots8 = LDMA_CH_CFG_ARBSLOTS_EIGHT,

        SourceIncrement = LDMA_CH_CFG_SRCINCSIGN_POSITIVE,
        SourceDecrement = LDMA_CH_CFG_SRCINCSIGN_NEGATIVE,

        DestinationIncrement = LDMA_CH_CFG_DSTINCSIGN_POSITIVE,
        DestinationDecrement = LDMA_CH_CFG_DSTINCSIGN_NEGATIVE,
    };

#if ADC_COUNT
    enum struct ADCSignal
    {
        Single = LDMA_SIGSEL(ADC0SINGLE),
        Scan = LDMA_SIGSEL(ADC0SCAN),
    };
#endif

#if IADC_COUNT
    enum struct IADCSignal
    {
        Single = LDMA_SIGSEL(IADC0IADC_SINGLE),
        Scan = LDMA_SIGSEL(IADC0IADC_SCAN),
    };
#endif

    enum struct USARTSignal
    {
        RxDataValid = LDMA_SIGSEL(USART0RXDATAV),
        TxFree = LDMA_SIGSEL(USART0TXBL),
        TxEmpty = LDMA_SIGSEL(USART0TXEMPTY),
        RxDataValidRight = LDMA_SIGSEL(USART1RXDATAVRIGHT),
        TxFreeRight = LDMA_SIGSEL(USART1TXBLRIGHT),
    };

#if UART_COUNT
    enum struct UARTSignal
    {
        RxDataValid = LDMA_SIGSEL(UART0RXDATAV),
        TxFree = LDMA_SIGSEL(UART0TXBL),
        TxEmpty = LDMA_SIGSEL(UART0TXEMPTY),
    };
#endif

#if LEUART_COUNT
    enum struct LEUARTSignal
    {
        RxDataValid = LDMA_SIGSEL(LEUART0RXDATAV),
        TxFree = LDMA_SIGSEL(LEUART0TXBL),
        TxEmpty = LDMA_SIGSEL(LEUART0TXEMPTY),
    };
#endif

    enum struct I2CSignal
    {
        RxDataValid = LDMA_SIGSEL(I2C0RXDATAV),
        TxFree = LDMA_SIGSEL(I2C0TXBL),
    };

    enum struct TIMERSignal
    {
        UnderOverflow = LDMA_SIGSEL(TIMER0UFOF),
        CC0 = LDMA_SIGSEL(TIMER0CC0),
        CC1 = LDMA_SIGSEL(TIMER0CC1),
        CC2 = LDMA_SIGSEL(TIMER0CC2),
#ifdef _LDMAXBAR_CH_REQSEL_SIGSEL_TIMER1CC3
        CC3 = LDMA_SIGSEL(TIMER1CC3),
#endif
    };

#if VDAC_COUNT
    enum VDACSignal
    {
        CH0 = LDMA_SIGSEL(VDAC0CH0),
        CH1 = LDMA_SIGSEL(VDAC0CH1),
    };
#endif

#if PDM_COUNT
    enum PDMSignal
    {
        RxDataValid = LDMA_SIGSEL(PDMRXDATAV),
    };
#endif

#undef LDMA_SOURCESEL
#undef LDMA_SIGSEL

};

//! A handle representing an LDMAChannel
class LDMAChannelHandle
{
private:
    constexpr LDMAChannelHandle(unsigned index) : index(index) {}

    unsigned index;

#ifdef LDMAXBAR
    volatile uint32_t& REQSEL() { return LDMAXBAR->CH[index].REQSEL; }
#else
    volatile uint32_t& REQSEL() { return Channel().REQSEL; }
#endif

public:
    constexpr LDMAChannelHandle() : index(~0u) {}

    //! Checks if the handle represents a valid LDMAChannel
    bool IsValid() const { return (int)index >= 0; }
    //! Gets the index of the LDMAChannel represented by this handle
    operator unsigned() const { return index; }
    //! Gets the LDMAChannel represented by this handle
    LDMAChannel& Channel();

    //! Sets the SYNC bit corresponding to the LDMAChannel represented by this handle
    void SetSync();
    //! Clears the SYNC bit corresponding to the LDMAChannel represented by this handle
    void ClearSync();
    //! Sets the SYNC bit corresponding to the LDMAChannel represented by this handle to the specified value
    void SetSync(bool value) { value ? SetSync() : ClearSync(); }

    //! Enables the LDMAChannel represented by this handle
    void Enable();
    //! Disables the LDMAChannel represented by this handle
    void Disable();
    //! Checks if the LDMAChannel represented by this handle is enabled
    bool IsEnabled() const;
    //! Checks if the LDMAChannel represented by this handle is currently transferring data
    bool IsBusy() const;

    //! Clears the DONE bit correspnding to the LDMAChannel represented by this handle
    void ClearDone();
    //! Sets the DONE bit correspnding to the LDMAChannel represented by this handle
    void SetDone();
    //! Checks if the LDMAChannel represented by this handle is done transferring data
    bool IsDone() const;

    //! Triggers a software request on the LDMAChannel represented by this handle
    void Request();
    //! Loads the LDMADescriptor at the address specified by the LINK register of the LDMAChannel represented by this handle
    void LinkLoad();
    //! Loads the specified LDMADescriptor to the LDMAChannel represented by this handle
    void LinkLoad(LDMADescriptor& dma);
    //! Clears a pending request on the LDMAChannel represented by this handle
    void RequestClear();

    //! Enables requests to be processed on the LDMAChannel represented by this handle
    void RequestsEnable();
    //! Prevents requests from being processed on the LDMAChannel represented by this handle
    void RequestsDisable();
    //! Checks if request processing is disabled on the LDMAChannel represented by this handle
    bool RequestsDisabled() const;
    //! Checks if there are requests pending on the LDMAChannel represented by this handle
    bool RequestsPending() const;

    //! Condfigures the LDMAChannel represented by this handle
    ALWAYS_INLINE void Setup(LDMAChannel::Flags flags) { Channel().CFG = flags; }
    //! Specified the number of loops for looping descriptors on the LDMAChannel represented by this handle
    ALWAYS_INLINE void Loop(uint32_t count) { Channel().LOOP = count; }
    //! Retrieves the primary LDMADescriptor of the LDMAChannel represented by this handle
    ALWAYS_INLINE volatile LDMADescriptor& RootDescriptor() { return Channel().Descriptor(); }
    //! Retrieves the pointer to the linked LDMADescriptor of the LDMAChannel represented by this handle
    ALWAYS_INLINE LDMADescriptor* LinkedDescriptor() { return (LDMADescriptor*)(Channel().LINK & ~3); }
    //! Waits for the DONE interrupt flag to be set on the LDMAChannel represented by this handle
    //! @note this flag is set by transfers having the Flags::SetDone flag, not when a transfer simply completes
    async(WaitForDoneFlag);

    //! Disables source for the LDMAChannel represented by this handle, allowing software triggered usage only
    ALWAYS_INLINE void SourceNone() { REQSEL() = 0; }
    //! Configures the LDMAChannel represented by this handle for the specified PRS channel
    ALWAYS_INLINE void SourcePRS(unsigned index) { ASSERT(index <= 1); REQSEL() = LDMAChannel::SourcePRS << 16 | index; }
#if ADC_COUNT
    //! Configures the LDMAChannel represented by this handle for the specified ADC peripheral and signal
    ALWAYS_INLINE void SourceADCChannel(unsigned index, LDMAChannel::ADCSignal sig) { ASSERT(index < ADC_COUNT); REQSEL() = (LDMAChannel::SourceADC0 + index) << 16 | uint32_t(sig); }
#endif
#if IADC_COUNT
    //! Configures the LDMAChannel represented by this handle for the specified IADC peripheral and signal
    ALWAYS_INLINE void SourceIADCChannel(unsigned index, LDMAChannel::IADCSignal sig) { ASSERT(index < IADC_COUNT); REQSEL() = (LDMAChannel::SourceIADC0 + index) << 16 | uint32_t(sig); }
#endif
    //! Configures the LDMAChannel represented by this handle for the specified USART peripheral and signal
    ALWAYS_INLINE void SourceUSARTChannel(unsigned index, LDMAChannel::USARTSignal sig) { ASSERT(index < USART_COUNT); REQSEL() = (LDMAChannel::SourceUSART0 + index) << 16 | uint32_t(sig); }
#if UART_COUNT
    //! Configures the LDMAChannel represented by this handle for the specified UART peripheral and signal
    ALWAYS_INLINE void SourceUARTChannel(unsigned index, LDMAChannel::UARTSignal sig) { ASSERT(index < UART_COUNT); REQSEL() = (LDMAChannel::SourceUART0 + index) << 16 | uint32_t(sig); }
#endif
#if LEUART_COUNT
    //! Configures the LDMAChannel represented by this handle for the specified LEUART peripheral and signal
    ALWAYS_INLINE void SourceLEUARTChannel(unsigned index, LDMAChannel::LEUARTSignal sig) { ASSERT(index < LEUART_COUNT); REQSEL() = (LDMAChannel::SourceLEUART0 + index) << 16 | uint32_t(sig); }
#endif
    //! Configures the LDMAChannel represented by this handle for the specified I2C peripheral and signal
    ALWAYS_INLINE void SourceI2CChannel(unsigned index, LDMAChannel::I2CSignal sig) { ASSERT(index < I2C_COUNT); REQSEL() = (LDMAChannel::SourceI2C0 + index) << 16 | uint32_t(sig); }
    //! Configures the LDMAChannel represented by this handle for the specified TIMER peripheral and signal
    ALWAYS_INLINE void SourceTIMERChannel(unsigned index, LDMAChannel::TIMERSignal sig) { ASSERT(index < TIMER_COUNT); REQSEL() = (LDMAChannel::SourceTIMER0 + index) << 16 | uint32_t(sig); }
#if VDAC_COUNT
    //! Configures the LDMAChannel represented by this handle for the specified VDAC peripheral and signal
    ALWAYS_INLINE void SourceVDACChannel(unsigned index, LDMAChannel::VDACSignal sig) { ASSERT(index < VDAC_COUNT); REQSEL() = (LDMAChannel::SourceVDAC0 + index) << 16 | uint32_t(sig); }
#endif

    friend class LDMAController;
    friend class LDMADescriptor;
};

DEFINE_FLAG_ENUM(LDMAChannel::Flags);

//! LDMA controller instance
class LDMAController : public LDMA_TypeDef
{
    friend class LDMAChannelHandle;

private:
    LDMAChannelHandle GetChannel(uint32_t srcDef, bool reuse = true);
#ifdef LDMAXBAR
    static volatile uint32_t& REQSEL(unsigned index) { return LDMAXBAR->CH[index].REQSEL; }
#else
    volatile uint32_t& REQSEL(unsigned index) { return CH[index].REQSEL; }
#endif

public:
    //! Enables the clock to the LDMA peripheral
    void EnableClock()
    {
        CMU->EnableLDMA();
#ifdef LDMA_EN_EN
        EN = LDMA_EN_EN;    // separate enable of the peripheral itself
#endif
    }

    //! Gets the LDMAChannelHandle for the channel with the specified index
    LDMAChannelHandle GetChannelByIndex(unsigned n) { return n; }
    //! Allocates a LDMAChannelHandle for a software triggered channel
    LDMAChannelHandle GetTriggeredChannel() { return GetChannel(LDMAChannel::SourceNone << 16 | 1, false); }
    //! Allocates a LDMAChannelHandle for the specified PRS channel, optionally reusing a previously allocated channel
    LDMAChannelHandle GetPRSChannel(unsigned index, bool reuse = true) { return GetChannel(LDMAChannel::SourcePRS << 16 | index, reuse); }
#if ADC_COUNT
    //! Allocates a LDMAChannelHandle for the specified ADC peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetADCChannel(unsigned index, LDMAChannel::ADCSignal sig, bool reuse = true) { ASSERT(index < ADC_COUNT); return GetChannel((LDMAChannel::SourceADC0 + index) << 16 | uint32_t(sig), reuse); }
#endif
#if IADC_COUNT
    //! Allocates a LDMAChannelHandle for the specified ADC peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetIADCChannel(unsigned index, LDMAChannel::IADCSignal sig, bool reuse = true) { ASSERT(index < IADC_COUNT); return GetChannel((LDMAChannel::SourceIADC0 + index) << 16 | uint32_t(sig), reuse); }
#endif
    //! Allocates a LDMAChannelHandle for the specified USART peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetUSARTChannel(unsigned index, LDMAChannel::USARTSignal sig, bool reuse = true) { ASSERT(index < USART_COUNT); return GetChannel((LDMAChannel::SourceUSART0 + index) << 16 | uint32_t(sig), reuse); }
#if UART_COUNT
    //! Allocates a LDMAChannelHandle for the specified UART peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetUARTChannel(uint index, LDMAChannel::UARTSignal sig, bool reuse = true) { ASSERT(index < UART_COUNT); return GetChannel((LDMAChannel::SourceUART0 + index) << 16 | uint32_t(sig), reuse); }
#endif
#if LEUART_COUNT
    //! Allocates a LDMAChannelHandle for the specified LEUART peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetLEUARTChannel(unsigned index, LDMAChannel::LEUARTSignal sig, bool reuse = true) { ASSERT(index < LEUART_COUNT); return GetChannel((LDMAChannel::SourceLEUART0 + index) << 16 | uint32_t(sig), reuse); }
#endif
    //! Allocates a LDMAChannelHandle for the specified I2C peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetI2CChannel(unsigned index, LDMAChannel::I2CSignal sig, bool reuse = true) { ASSERT(index < I2C_COUNT); return GetChannel((LDMAChannel::SourceI2C0 + index) << 16 | uint32_t(sig), reuse); }
    //! Allocates a LDMAChannelHandle for the specified TIMER peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetTIMERChannel(unsigned index, LDMAChannel::TIMERSignal sig, bool reuse = true) { ASSERT(index < TIMER_COUNT); return GetChannel((LDMAChannel::SourceTIMER0 + index) << 16 | uint32_t(sig), reuse); }
#if VDAC_COUNT
    //! Allocates a LDMAChannelHandle for the specified VDAC peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetVDACChannel(unsigned index, LDMAChannel::VDACSignal sig, bool reuse = true) { ASSERT(index < VDAC_COUNT); return GetChannel((LDMAChannel::SourceVDAC0 + index) << 16 | uint32_t(sig), reuse); }
#endif
#if PDM_COUNT
    //! Allocates a LDMAChannelHandle for the specified PDM peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetPDMChannel(unsigned index, LDMAChannel::PDMSignal sig, bool reuse = true) { ASSERT(index < PDM_COUNT); return GetChannel((LDMAChannel::SourcePDM0 + index) << 16 | uint32_t(sig), reuse); }
#endif

    //! Gets the number of unused channels that can be still allocated
    unsigned FreeChannels();
    //! Waits for all the channels specified in the mask to set their done flag
    async(WaitForDoneMask, uint32_t mask);
};

ALWAYS_INLINE LDMAChannel& LDMAChannelHandle::Channel() { return *(LDMAChannel*)&LDMA->CH[index]; }

#ifdef _LDMA_SYNCSWSET_MASK
ALWAYS_INLINE void LDMAChannelHandle::SetSync() { LDMA->SYNCSWSET = BIT(index); }
ALWAYS_INLINE void LDMAChannelHandle::ClearSync() { LDMA->SYNCSWCLR = BIT(index); }
#else
ALWAYS_INLINE void LDMAChannelHandle::SetSync() { EFM32_BITSET_REG(LDMA->SYNC, BIT(index)); }
ALWAYS_INLINE void LDMAChannelHandle::ClearSync() { EFM32_BITCLR_REG(LDMA->SYNC, BIT(index)); }
#endif

ALWAYS_INLINE void LDMAChannelHandle::Enable() { EFM32_BITSET_REG(LDMA->CHEN, BIT(index)); }
ALWAYS_INLINE void LDMAChannelHandle::Disable() { EFM32_BITCLR_REG(LDMA->CHEN, BIT(index)); }
ALWAYS_INLINE bool LDMAChannelHandle::IsEnabled() const { return GETBIT(LDMA->CHEN, index); }

ALWAYS_INLINE bool LDMAChannelHandle::IsBusy() const { return GETBIT(LDMA->CHBUSY, index); }

ALWAYS_INLINE void LDMAChannelHandle::ClearDone() { EFM32_BITCLR_REG(LDMA->CHDONE, BIT(index)); EFM32_IFC(LDMA) = BIT(index); }
ALWAYS_INLINE void LDMAChannelHandle::SetDone() { EFM32_BITSET_REG(LDMA->CHDONE, BIT(index)); EFM32_IFS(LDMA) = BIT(index); }
ALWAYS_INLINE bool LDMAChannelHandle::IsDone() const { return GETBIT(LDMA->CHDONE, index); }

ALWAYS_INLINE void LDMAChannelHandle::Request() { LDMA->SWREQ = BIT(index); }
ALWAYS_INLINE void LDMAChannelHandle::LinkLoad(LDMADescriptor& desc) { RootDescriptor().LINK = (uint32_t)&desc; LDMA->LINKLOAD = BIT(index); }
ALWAYS_INLINE void LDMAChannelHandle::LinkLoad() { LDMA->LINKLOAD = BIT(index); }
ALWAYS_INLINE void LDMAChannelHandle::RequestClear() { LDMA->REQCLEAR = BIT(index); }

ALWAYS_INLINE void LDMAChannelHandle::RequestsEnable() { EFM32_BITCLR_REG(LDMA->REQDIS, BIT(index)); }
ALWAYS_INLINE void LDMAChannelHandle::RequestsDisable() { EFM32_BITSET_REG(LDMA->REQDIS, BIT(index)); }
ALWAYS_INLINE bool LDMAChannelHandle::RequestsDisabled() const { return !GETBIT(LDMA->REQDIS, index); }

ALWAYS_INLINE bool LDMAChannelHandle::RequestsPending() const { return GETBIT(LDMA->REQPEND, index); }

ALWAYS_INLINE LDMAChannelHandle LDMADescriptor::ChannelHandle() volatile const { return ((uintptr_t)this - (uintptr_t)LDMA->CH) / sizeof(LDMA_CH_TypeDef); }

ALWAYS_INLINE async(LDMAChannelHandle::WaitForDoneFlag) { return async_forward(LDMA->WaitForDoneMask, BIT(index)); }
