/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * lib-efm32/targets/efm32-series1/hw/I2C.h
 */

#pragma once

#include <kernel/kernel.h>

#include <base/Span.h>

#include <hw/GPIO.h>

#ifdef I2C0
#undef I2C0
#define I2C0    CM_PERIPHERAL(I2C, I2C0_BASE)
#endif

#ifdef I2C1
#undef I2C1
#define I2C1    CM_PERIPHERAL(I2C, I2C1_BASE)
#endif

class I2C : public I2C_TypeDef
{
public:
    enum Flags
    {
        Enable = I2C_CTRL_EN,
        SlaveEnable = I2C_CTRL_SLAVE,

        AutoACK = I2C_CTRL_AUTOACK,
        AutoStopWhenEmpty = I2C_CTRL_AUTOSE,
        AutoStopOnNAK = I2C_CTRL_AUTOSN,

        GeneralCallDisable = 0,
        GeneralCallEnable = I2C_CTRL_GCAMEN,

        TxInterruptLevelEmpty = I2C_CTRL_TXBIL_EMPTY,
        TxInterruptLevelHalfFull = I2C_CTRL_TXBIL_HALFFULL,

        ClockRatio4_4 = I2C_CTRL_CLHR_STANDARD,
        ClockRatio6_3 = I2C_CTRL_CLHR_ASYMMETRIC,
        ClockRatio11_6 = I2C_CTRL_CLHR_FAST,

        BusIdleTimeoutOff = I2C_CTRL_BITO_OFF,
        BusIdleTimeout40 = I2C_CTRL_BITO_40PCC,
        BusIdleTimeout80 = I2C_CTRL_BITO_80PCC,
        BusIdleTimeout160 = I2C_CTRL_BITO_160PCC,

        BusIdleTimeoutEnable = I2C_CTRL_GIBITO,

        ClockLowTimeoutOff = I2C_CTRL_CLTO_OFF,
        ClockLowTimeout40 = I2C_CTRL_CLTO_40PCC,
        ClockLowTimeout80 = I2C_CTRL_CLTO_80PCC,
        ClockLowTimeout160 = I2C_CTRL_CLTO_160PCC,
        ClockLowTimeout320 = I2C_CTRL_CLTO_320PCC,
        ClockLowTimeout1024 = I2C_CTRL_CLTO_1024PCC,
    };

    enum BusState
    {
        BusIdle, BusWait, BusStart, BusAddr, BusAddrAck, BusData, BusDataAck,
    };

    union StateFlags
    {
        constexpr StateFlags(uint32_t value) : flags(value) {}

        uint32_t flags;
        struct
        {
            uint32_t start : 1;
            uint32_t repeatedStart : 1;
            uint32_t address : 1;
            uint32_t complete : 1;
            uint32_t bufferLevel : 1;
            uint32_t dataValid : 1;
            uint32_t ack : 1;
            uint32_t nack : 1;
            uint32_t masterStop : 1;
            uint32_t arbitrationLost : 1;
            uint32_t busError : 1;
            uint32_t busHeld : 1;
            uint32_t txOverflow : 1;
            uint32_t rxUnderflow : 1;
            uint32_t busIdleTimeout : 1;
            uint32_t clockLowTimeout : 1;
            uint32_t slaveStop : 1;
            uint32_t rxFull : 1;
            uint32_t clockError : 1;
            uint32_t : 13;
        };
        struct
        {
            uint32_t : 6;
            uint32_t ackOrNak : 2;
            uint32_t : 24;
        };

        constexpr operator bool() const { return flags & ~(I2C_IF_START | I2C_IF_RSTART | I2C_IF_TXBL | I2C_IF_TXC | I2C_IF_SSTOP | I2C_IF_MSTOP | I2C_IF_BUSHOLD); }
    };

    //! Gets the index of the peripheral
    ALWAYS_INLINE int Index() const { return ((unsigned)this >> 10) & 0xF; }

    //! Enables peripheral clock
	void EnableClock() { CMU->EnableI2C(Index()); }
    //! Configures the peripheral
    void Setup(Flags flags) { CTRL = flags; }
    //! Gets the bit period in clocks, depending on the configured flags
    int ClockPeriod() { return BYTES(8, 9, 17)[(CTRL & _I2C_CTRL_CLHR_MASK) >> _I2C_CTRL_CLHR_SHIFT]; }
    void OutputFrequency(uint32_t freq, int clkper = 0)
    {
        if (clkper == 0)
            clkper = ClockPeriod();
        CLKDIV = (CMU->GetCoreFrequency() / freq - clkper + 1) / clkper;
    }

    IRQn_Type IRQn() const { return LOOKUP_TABLE(IRQn_Type, I2C0_IRQn, I2C1_IRQn)[Index()]; }

    void IRQEnable() { NVIC_EnableIRQ(IRQn()); }
    void IRQDisable() { NVIC_DisableIRQ(IRQn()); }
    void IRQClear() { NVIC_ClearPendingIRQ(IRQn()); }

    void ClearPending() { CMD = I2C_CMD_CLEARPC; }
    void TxClearBuffer() { CMD = I2C_CMD_CLEARTX; }
    void Continue() { CMD = I2C_CMD_CONT; }
    void NACK() { CMD = I2C_CMD_NACK; }
    void ACK() { CMD = I2C_CMD_ACK; }
    void Stop() { CMD = I2C_CMD_STOP; }
    void Start() { CMD = I2C_CMD_START; }
    void Abort() { CMD = I2C_CMD_ABORT; }

    BusState State() { return (BusState)((STATE & _I2C_STATE_STATE_MASK) >> _I2C_STATE_STATE_SHIFT); }
    bool BusHeld() { return STATE & I2C_STATE_BUSHOLD; }
    bool NACKed() { return STATE & I2C_STATE_NACKED; }
    bool IsTransmitter() { return STATE & I2C_STATE_TRANSMITTER; }
    bool IsMaster() { return STATE & I2C_STATE_MASTER; }
    bool BusBusy() { return STATE & I2C_STATE_BUSY; }

    bool RxEmpty() { return !(STATUS & I2C_STATUS_RXDATAV); }
    bool RxFull() { return STATUS & I2C_STATUS_RXFULL; }
    void RxClearBuffer() { while (!RxEmpty()) Receive(); }

    bool TxDone() { return STATUS & I2C_STATUS_TXC; }
    bool TxEmpty() { return STATUS & I2C_STATUS_TXBL; }

    bool WillStart() { return STATUS & I2C_STATUS_PSTART; }
    bool WillStop() { return STATUS & I2C_STATUS_PSTOP; }
    bool WillACK() { return STATUS & I2C_STATUS_PACK; }
    bool WillNACK() { return STATUS & I2C_STATUS_PNACK; }
    bool WillContinue() { return STATUS & I2C_STATUS_PCONT; }
    bool WillAbort() { return STATUS & I2C_STATUS_PABORT; }

    StateFlags ClearFlags(uint ignore = 0) { return IFC & ~ignore; }

    void Send(uint b) { TXDATA = b; }
    uint Receive() { return RXDATA; }
    bool CanStart() { auto state = State(); return (state == BusIdle || state == BusDataAck); }

#ifdef EFM32_GPIO_LINEAR_INDEX
    void ConfigureScl(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::WiredAnd) { pin.ConfigureAlternate(mode, ROUTEPEN, 1, 1u); }
    void ConfigureSda(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::WiredAnd) { pin.ConfigureAlternate(mode, ROUTEPEN, 0, 0u); }
#endif

    bool Idle();

    //! Resets the bus
    async(Reset);

    //! Starts or restarts a read transaction, reading the specified number of bytes from the bus
    async(Read, uint8_t address, Buffer data, bool start, bool stop) { return async_forward(_Read, Operation(address, true, start, stop, data.Length()), data.Pointer()); }
    //! Continues a read transaction, reading the specified number of bytes from the bus
    async(Read, Buffer data, bool stop) { return async_forward(_Read, Operation(true, stop, data.Length()), data.Pointer()); }

    //! Starts or restarts a write transaction, writing the specified number of bytes to the bus
    async(Write, uint8_t address, Span data, bool start, bool stop) { return async_forward(_Write, Operation(address, false, start, stop, data.Length()), data.Pointer()); }
    //! Continues a write transaction, writing the specified number of bytes to the bus
    async(Write, Span data, bool stop) { return async_forward(_Write, Operation(false, stop, data.Length()), data.Pointer()); }

private:
    enum InterruptFlags
    {
        AwaitFlagsNoAck = I2C_IF_BUSERR | I2C_IF_ARBLOST | I2C_IF_RXDATAV | I2C_IF_MSTOP,
        AwaitFlags = AwaitFlagsNoAck | I2C_IF_ACK | I2C_IF_NACK,
    };

    union Operation
    {
        constexpr Operation(uint8_t address, bool read, bool start, bool stop, uint16_t length)
            : value(address << 25 | read * BIT(24) | stop * BIT(17) | start * BIT(16) | length) {}
        constexpr Operation(bool read, bool stop, uint16_t length)
            : value(read << 24 | BIT(18) | stop * BIT(17) | length) {}

        uint32_t value;
        struct
        {
            uint16_t length;
            bool start : 1;
            bool stop : 1;
            bool noAddress : 1;
            uint8_t : 5;
            union
            {
                struct
                {
                    bool read : 1;
                    uint8_t address : 7;
                };
                uint8_t fullAddress;
            };
        };
    };

    async(_Address, Operation op);
    async(_Read, Operation op, char* data);
    async(_Write, Operation op, const char* data);

    void TransactionInit();
    void TransactionCleanup();
    bool HandleError(StateFlags flags);
    async(OnUnhandledErrorAsync, StateFlags flags);
};

DEFINE_FLAG_ENUM(I2C::Flags);
