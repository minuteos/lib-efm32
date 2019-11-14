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
        Next = 0x13,
        Previous = 0xFFFFFFF3,
    };

    //! Creates an LDMA LINK from an absolute pointer
    constexpr LDMALink(class LDMADescriptor* ptr) : value(ptr ? (uint32_t)ptr | 2 : 0) {}
    //! Creates an LDMA LINK from one of the fixed values
    constexpr LDMALink(LDMALink::FixedValue value) : value((uint32_t)value) {}

private:
    uint32_t value;

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

        SetDone = LDMA_CH_CTRL_DONEIFSEN,

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
        M2M = BlockSize1 | SourceIncrement1 | DestinationIncrement1,
        M2P = BlockSize1 | SourceIncrement1 | DestinationIncrement0,
        P2M = BlockSize1 | SourceIncrement0 | DestinationIncrement1,
        P2P = BlockSize1 | SourceIncrement0 | DestinationIncrement0,
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
    ALWAYS_INLINE bool IsTransfer() const { return (CTRL & _LDMA_CH_CTRL_STRUCTTYPE_MASK) == TypeTransfer; }
    //! Checks if the descriptor specified an immediate write operation
    ALWAYS_INLINE bool IsImmediate() const { return (CTRL & _LDMA_CH_CTRL_STRUCTTYPE_MASK) == TypeImmediate; }
    //! Checks if the descriptor specified a synchronization operation
    ALWAYS_INLINE bool IsSync() const { return (CTRL & _LDMA_CH_CTRL_STRUCTTYPE_MASK) == TypeSync; }

    //! Gets the source address of the transfer
    ALWAYS_INLINE volatile const void* Source() const { return (const volatile void*)SRC; }
    //! Sets the source address of the transfer
    ALWAYS_INLINE void Source(volatile const void* ptr) { SRC = (uint32_t)ptr; }
    //! Gets the destination address of the transfer
    ALWAYS_INLINE volatile void* Destination() const { return (volatile void*)DST; }
    //! Sets the destination address of the transfer
    ALWAYS_INLINE void Destination(volatile void* ptr) { DST = (uint32_t)ptr; }
    //! Gets the number of units transfered (depends on Flags)
    ALWAYS_INLINE size_t Count() const { return ((CTRL & _LDMA_CH_CTRL_XFERCNT_MASK) >> _LDMA_CH_CTRL_XFERCNT_SHIFT) + 1; }
    //! Sets the number of units transfered (depends on Flags)
    ALWAYS_INLINE void Count(size_t val) { MODMASK_SAFE(CTRL, _LDMA_CH_CTRL_XFERCNT_MASK, (val - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT); }
    //! Sets the linked descriptor to be used after this one completes
    ALWAYS_INLINE void Link(LDMALink link) { LINK = link.value; }

    //! Gets the LDMAChannel to which this descriptor belongs
    //! @warning Can be used only on the primary descriptor obtained from an LDMAChannel
    ALWAYS_INLINE class LDMAChannel& Channel() { class LDMAChannel* ch = (LDMAChannel*)((uintptr_t)this - offsetof(LDMA_CH_TypeDef, CTRL)); return *ch; }
    //! Gets the LDMAChannelHandle of the channel to which this descriptor belongs
    //! @warning Can be used only on the primary descriptor obtained from an LDMAChannel
    class LDMAChannelHandle ChannelHandle();

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
    enum Source
    {
        SourceNone =    _LDMA_CH_REQSEL_SOURCESEL_NONE,
        SourcePRS =     _LDMA_CH_REQSEL_SOURCESEL_PRS,
        SourceADC0 =    _LDMA_CH_REQSEL_SOURCESEL_ADC0,
#ifdef _LDMA_CH_REQSEL_SOURCESEL_ADC1
        SourceADC1 =    _LDMA_CH_REQSEL_SOURCESEL_ADC1,
#endif
#ifdef _LDMA_CH_REQSEL_SOURCESEL_VDAC0
        SourceVDAC0 =   _LDMA_CH_REQSEL_SOURCESEL_VDAC0,
#endif
        SourceUSART0 =  _LDMA_CH_REQSEL_SOURCESEL_USART0,
        SourceUSART1 =  _LDMA_CH_REQSEL_SOURCESEL_USART1,
#ifdef _LDMA_CH_REQSEL_SOURCESEL_USART2
        SourceUSART2 =  _LDMA_CH_REQSEL_SOURCESEL_USART2,
#endif
#ifdef _LDMA_CH_REQSEL_SOURCESEL_USART3
        SourceUSART3 =  _LDMA_CH_REQSEL_SOURCESEL_USART3,
#endif
#ifdef _LDMA_CH_REQSEL_SOURCESEL_USART4
        SourceUSART4 =  _LDMA_CH_REQSEL_SOURCESEL_USART4,
#endif
#ifdef _LDMA_CH_REQSEL_SOURCESEL_USART5
        SourceUSART5 =  _LDMA_CH_REQSEL_SOURCESEL_USART5,
#endif
#ifdef _LDMA_CH_REQSEL_SOURCESEL_UART1
        SourceUART0 =   _LDMA_CH_REQSEL_SOURCESEL_UART0,
        SourceUART1 =   _LDMA_CH_REQSEL_SOURCESEL_UART1,
#endif
        SourceLEUART0 = _LDMA_CH_REQSEL_SOURCESEL_LEUART0,
#ifdef _LDMA_CH_REQSEL_SOURCESEL_LEUART1
        SourceLEUART1 = _LDMA_CH_REQSEL_SOURCESEL_LEUART1,
#endif
        SourceI2C0 =    _LDMA_CH_REQSEL_SOURCESEL_I2C0,
#ifdef _LDMA_CH_REQSEL_SOURCESEL_I2C2
        SourceI2C1 =    _LDMA_CH_REQSEL_SOURCESEL_I2C1,
        SourceI2C2 =    _LDMA_CH_REQSEL_SOURCESEL_I2C2,
#endif
        SourceTIMER0 =  _LDMA_CH_REQSEL_SOURCESEL_TIMER0,
        SourceTIMER1 =  _LDMA_CH_REQSEL_SOURCESEL_TIMER1,
#ifdef _LDMA_CH_REQSEL_SOURCESEL_TIMER6
        SourceTIMER2 =  _LDMA_CH_REQSEL_SOURCESEL_TIMER2,
        SourceTIMER3 =  _LDMA_CH_REQSEL_SOURCESEL_TIMER3,
        SourceTIMER4 =  _LDMA_CH_REQSEL_SOURCESEL_TIMER4,
        SourceTIMER5 =  _LDMA_CH_REQSEL_SOURCESEL_TIMER5,
        SourceTIMER6 =  _LDMA_CH_REQSEL_SOURCESEL_TIMER6,
#endif
        SourceMSC    =  _LDMA_CH_REQSEL_SOURCESEL_MSC,
#ifdef _LDMA_CH_REQSEL_SOURCESEL_CRYPTO0
        SourceCRYPTO =  _LDMA_CH_REQSEL_SOURCESEL_CRYPTO0,
#elif defined(_LDMA_CH_REQSEL_SOURCESEL_CRYPTO)
        SourceCRYPTO =  _LDMA_CH_REQSEL_SOURCESEL_CRYPTO,
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

    enum struct ADCSignal
    {
        Single = LDMA_CH_REQSEL_SIGSEL_ADC0SINGLE,
        Scan = LDMA_CH_REQSEL_SIGSEL_ADC0SCAN,
    };

    enum struct USARTSignal
    {
        RxDataValid = LDMA_CH_REQSEL_SIGSEL_USART0RXDATAV,
        TxFree = LDMA_CH_REQSEL_SIGSEL_USART0TXBL,
        TxEmpty = LDMA_CH_REQSEL_SIGSEL_USART0TXEMPTY,
        RxDataValidRight = LDMA_CH_REQSEL_SIGSEL_USART1RXDATAVRIGHT,
        TxFreeRight = LDMA_CH_REQSEL_SIGSEL_USART1TXBLRIGHT,
    };

#if UART_COUNT
    enum struct UARTSignal
    {
        RxDataValid = LDMA_CH_REQSEL_SIGSEL_UART0RXDATAV,
        TxFree = LDMA_CH_REQSEL_SIGSEL_UART0TXBL,
        TxEmpty = LDMA_CH_REQSEL_SIGSEL_UART0TXEMPTY,
    };
#endif

    enum struct LEUARTSignal
    {
        RxDataValid = LDMA_CH_REQSEL_SIGSEL_LEUART0RXDATAV,
        TxFree = LDMA_CH_REQSEL_SIGSEL_LEUART0TXBL,
        TxEmpty = LDMA_CH_REQSEL_SIGSEL_LEUART0TXEMPTY,
    };

    enum struct I2CSignal
    {
        RxDataValid = LDMA_CH_REQSEL_SIGSEL_I2C0RXDATAV,
        TxFree = LDMA_CH_REQSEL_SIGSEL_I2C0TXBL,
    };

    enum struct TIMERSignal
    {
        UnderOverflow = LDMA_CH_REQSEL_SIGSEL_TIMER0UFOF,
        CC0 = LDMA_CH_REQSEL_SIGSEL_TIMER0CC0,
        CC1 = LDMA_CH_REQSEL_SIGSEL_TIMER0CC1,
        CC2 = LDMA_CH_REQSEL_SIGSEL_TIMER0CC2,
        CC3 = LDMA_CH_REQSEL_SIGSEL_TIMER1CC3,
    };

    enum VDACSignal
    {
        CH0 = LDMA_CH_REQSEL_SIGSEL_VDAC0CH0,
        CH1 = LDMA_CH_REQSEL_SIGSEL_VDAC0CH1,
    };
};

//! A handle representing an LDMAChannel
class LDMAChannelHandle
{
private:
    constexpr LDMAChannelHandle(unsigned index) : index(index) {}

    unsigned index;

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
    ALWAYS_INLINE LDMADescriptor& RootDescriptor() { return Channel().Descriptor(); }
    //! Waits for the DONE interrupt flag to be set on the LDMAChannel represented by this handle
    //! @note this flag is set by transfers having the Flags::SetDone flag, not when a transfer simply completes
    async(WaitForDoneFlag);

    //! Disables source for the LDMAChannel represented by this handle, allowing software triggered usage only
    ALWAYS_INLINE void SourceNone() { Channel().REQSEL = 0; }
    //! Configures the LDMAChannel represented by this handle for the specified PRS channel
    ALWAYS_INLINE void SourcePRS(unsigned index) { ASSERT(index <= 1); Channel().REQSEL = LDMAChannel::SourcePRS << 16 | index; }
    //! Configures the LDMAChannel represented by this handle for the specified ADC peripheral and signal
    ALWAYS_INLINE void SourceADCChannel(unsigned index, LDMAChannel::ADCSignal sig) { ASSERT(index < ADC_COUNT); Channel().REQSEL = (LDMAChannel::SourceADC0 + index) << 16 | uint32_t(sig); }
    //! Configures the LDMAChannel represented by this handle for the specified USART peripheral and signal
    ALWAYS_INLINE void SourceUSARTChannel(unsigned index, LDMAChannel::USARTSignal sig) { ASSERT(index < USART_COUNT); Channel().REQSEL = (LDMAChannel::SourceUSART0 + index) << 16 | uint32_t(sig); }
#if UART_COUNT
    //! Configures the LDMAChannel represented by this handle for the specified UART peripheral and signal
    ALWAYS_INLINE void SourceUARTChannel(unsigned index, LDMAChannel::UARTSignal sig) { ASSERT(index < UART_COUNT); Channel().REQSEL = (LDMAChannel::SourceUART0 + index) << 16 | uint32_t(sig); }
#endif
    //! Configures the LDMAChannel represented by this handle for the specified LEUART peripheral and signal
    ALWAYS_INLINE void SourceLEUARTChannel(unsigned index, LDMAChannel::LEUARTSignal sig) { ASSERT(index < LEUART_COUNT); Channel().REQSEL = (LDMAChannel::SourceLEUART0 + index) << 16 | uint32_t(sig); }
    //! Configures the LDMAChannel represented by this handle for the specified I2C peripheral and signal
    ALWAYS_INLINE void SourceI2CChannel(unsigned index, LDMAChannel::I2CSignal sig) { ASSERT(index < I2C_COUNT); Channel().REQSEL = (LDMAChannel::SourceI2C0 + index) << 16 | uint32_t(sig); }
    //! Configures the LDMAChannel represented by this handle for the specified TIMER peripheral and signal
    ALWAYS_INLINE void SourceTIMERChannel(unsigned index, LDMAChannel::TIMERSignal sig) { ASSERT(index < TIMER_COUNT); Channel().REQSEL = (LDMAChannel::SourceTIMER0 + index) << 16 | uint32_t(sig); }
#if VDAC_COUNT
    //! Configures the LDMAChannel represented by this handle for the specified VDAC peripheral and signal
    ALWAYS_INLINE void SourceVDACChannel(unsigned index, LDMAChannel::VDACSignal sig) { ASSERT(index < VDAC_COUNT); Channel().REQSEL = (LDMAChannel::SourceVDAC0 + index) << 16 | uint32_t(sig); }
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

public:
    //! Enables the clock to the LDMA peripheral
    void EnableClock() const { CMU->EnableLDMA(); }

    //! Gets the LDMAChannelHandle for the channel with the specified index
    LDMAChannelHandle GetChannelByIndex(unsigned n) { return n; }
    //! Allocates a LDMAChannelHandle for the specified PRS channel, optionally reusing a previously allocated channel
    LDMAChannelHandle GetPRSChannel(unsigned index, bool reuse = true) { return GetChannel(LDMAChannel::SourcePRS << 16 | index, reuse); }
    //! Allocates a LDMAChannelHandle for the specified ADC peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetADCChannel(unsigned index, LDMAChannel::ADCSignal sig, bool reuse = true) { ASSERT(index < ADC_COUNT); return GetChannel((LDMAChannel::SourceADC0 + index) << 16 | uint32_t(sig), reuse); }
    //! Allocates a LDMAChannelHandle for the specified USART peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetUSARTChannel(unsigned index, LDMAChannel::USARTSignal sig, bool reuse = true) { ASSERT(index < USART_COUNT); return GetChannel((LDMAChannel::SourceUSART0 + index) << 16 | uint32_t(sig), reuse); }
#if UART_COUNT
    //! Allocates a LDMAChannelHandle for the specified UART peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetUARTChannel(uint index, LDMAChannel::UARTSignal sig, bool reuse = true) { ASSERT(index < UART_COUNT); return GetChannel((LDMAChannel::SourceUART0 + index) << 16 | uint32_t(sig), reuse); }
#endif
    //! Allocates a LDMAChannelHandle for the specified LEUART peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetLEUARTChannel(unsigned index, LDMAChannel::LEUARTSignal sig, bool reuse = true) { ASSERT(index < LEUART_COUNT); return GetChannel((LDMAChannel::SourceLEUART0 + index) << 16 | uint32_t(sig), reuse); }
    //! Allocates a LDMAChannelHandle for the specified I2C peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetI2CChannel(unsigned index, LDMAChannel::I2CSignal sig, bool reuse = true) { ASSERT(index < I2C_COUNT); return GetChannel((LDMAChannel::SourceI2C0 + index) << 16 | uint32_t(sig), reuse); }
    //! Allocates a LDMAChannelHandle for the specified TIMER peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetTIMERChannel(unsigned index, LDMAChannel::TIMERSignal sig, bool reuse = true) { ASSERT(index < TIMER_COUNT); return GetChannel((LDMAChannel::SourceTIMER0 + index) << 16 | uint32_t(sig), reuse); }
#if VDAC_COUNT
    //! Allocates a LDMAChannelHandle for the specified VDAC peripheral and signal, optionally reusing a previously allocated channel
    LDMAChannelHandle GetVDACChannel(unsigned index, LDMAChannel::VDACSignal sig, bool reuse = true) { ASSERT(index < VDAC_COUNT); return GetChannel((LDMAChannel::SourceVDAC0 + index) << 16 | uint32_t(sig), reuse); }
#endif

    //! Gets the number of unused channels that can be still allocated
    unsigned FreeChannels() const;
    //! Waits for all the channels specified in the mask to set their done flag
    async(WaitForDoneMask, uint32_t mask);
};

ALWAYS_INLINE LDMAChannel& LDMAChannelHandle::Channel() { return *(LDMAChannel*)&LDMA->CH[index]; }

ALWAYS_INLINE void LDMAChannelHandle::SetSync() { EFM32_BITSET(LDMA->SYNC, BIT(index)); }
ALWAYS_INLINE void LDMAChannelHandle::ClearSync() { EFM32_BITCLR(LDMA->SYNC, BIT(index)); }

ALWAYS_INLINE void LDMAChannelHandle::Enable() { EFM32_BITSET(LDMA->CHEN, BIT(index)); }
ALWAYS_INLINE void LDMAChannelHandle::Disable() { EFM32_BITCLR(LDMA->CHEN, BIT(index)); }
ALWAYS_INLINE bool LDMAChannelHandle::IsEnabled() const { return GETBIT(LDMA->CHEN, index); }

ALWAYS_INLINE bool LDMAChannelHandle::IsBusy() const { return GETBIT(LDMA->CHBUSY, index); }

ALWAYS_INLINE void LDMAChannelHandle::ClearDone() { EFM32_BITCLR(LDMA->CHDONE, BIT(index)); }
ALWAYS_INLINE bool LDMAChannelHandle::IsDone() const { return GETBIT(LDMA->CHDONE, index); }

ALWAYS_INLINE void LDMAChannelHandle::Request() { LDMA->SWREQ = BIT(index); }
ALWAYS_INLINE void LDMAChannelHandle::LinkLoad(LDMADescriptor& desc) { RootDescriptor().LINK = (uint32_t)&desc; LDMA->LINKLOAD = BIT(index); }
ALWAYS_INLINE void LDMAChannelHandle::LinkLoad() { LDMA->LINKLOAD = BIT(index); }
ALWAYS_INLINE void LDMAChannelHandle::RequestClear() { LDMA->REQCLEAR = BIT(index); }

ALWAYS_INLINE void LDMAChannelHandle::RequestsEnable() { EFM32_BITCLR(LDMA->REQDIS, BIT(index)); }
ALWAYS_INLINE void LDMAChannelHandle::RequestsDisable() { EFM32_BITSET(LDMA->REQDIS, BIT(index)); }
ALWAYS_INLINE bool LDMAChannelHandle::RequestsDisabled() const { return !GETBIT(LDMA->REQDIS, index); }

ALWAYS_INLINE bool LDMAChannelHandle::RequestsPending() const { return GETBIT(LDMA->REQPEND, index); }

ALWAYS_INLINE LDMAChannelHandle LDMADescriptor::ChannelHandle() { return ((uintptr_t)this - (uintptr_t)LDMA->CH) / sizeof(LDMA_CH_TypeDef); }

ALWAYS_INLINE async(LDMAChannelHandle::WaitForDoneFlag) { return async_forward(LDMA->WaitForDoneMask, BIT(index)); }
