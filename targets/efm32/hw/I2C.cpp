/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * lib-efm32/targets/efm32-series1/hw/I2C.cpp
 */

#include "I2C.h"

#define DBGERR(error)	DBGCL(Index() ? "I2C1" : "I2C0", error ": %s%s%s%s%s%s %X %X %X", \
    STRINGS("IDLE", "WAIT", "START", "ADDR", "ADDRACK", "DATA", "DATAACK", "???")[(STATE >> 5) & 7], \
    STATE & I2C_STATE_BUSHOLD ? ",HLD" : "", \
    STATE & I2C_STATE_NACKED ? ",NAK" : "", \
    STATE & I2C_STATE_TRANSMITTER ? ",TX" : "", \
    STATE & I2C_STATE_MASTER ? ",MAS" : "", \
    STATE & I2C_STATE_BUSY ? ",BSY" : "", \
    STATUS, flags, IF);

#define DIAG_TRANS      1
#define DIAG_READ       2
#define DIAG_WRITE      4
#define DIAG_DATA       8
#define DIAG_ACK        16
#define DIAG_SLAVE      32

//#define EFM32_I2C_DEBUG   DIAG_TRANS

#if EFM32_I2C_DEBUG
#define DIAG(mask, ...)	if (((EFM32_I2C_DEBUG) & (mask)) == (mask)) { DBGCL(Index() ? "I2C1" : "I2C0", __VA_ARGS__); }
#else
#define DIAG(...)
#endif

#ifndef I2C_TIMEOUT
#define I2C_TIMEOUT	1000		// timeouts shouldn't normally occur
#endif

static uint32_t s_locks;

async(I2C::Reset)
async_def(int retry)
{
    if (State() == BusIdle || State() == BusDataAck)
    {
        ClearFlags();
        async_return(true);
    }

    StateFlags flags = ClearFlags();
    DBGERR("resetting hung bus");
    // re-enable with temporarily disabled arbitration so that we can fake start-stop conditions
#ifdef I2C_EN_EN
    EN = 0;
    CTRL_SET = I2C_CTRL_ARBDIS;
    EN = I2C_EN_EN;
#else
    CTRL &= ~I2C_CTRL_EN;
    CTRL |= I2C_CTRL_EN | I2C_CTRL_ARBDIS;
#endif
    Abort();
    flags = ClearFlags();

    for (f.retry = 0; f.retry < 10; f.retry++)
    {
        // try to send START-STOP 10x to release any hung slaves
        CMD = I2C_CMD_START | I2C_CMD_STOP;
        if (!await_mask_ms(STATUS, I2C_STATUS_PSTART | I2C_STATUS_PSTOP, 0, I2C_TIMEOUT))
        {
            flags = ClearFlags();
            DBGERR("error during bus reset");
            Abort();
            await_mask_ms(STATUS, I2C_STATUS_PSTART | I2C_STATUS_PSTOP, 0, I2C_TIMEOUT);
        }
    }

    // re-enable arbitration, the bus now has to be idle
    EFM32_BITCLR_REG(CTRL, I2C_CTRL_ARBDIS);
    flags = ClearFlags();
    DBGERR("bus reset complete");
}
async_end

void I2C::TransactionInit()
{
    ASSERT(IEN == 0);

    Cortex_SetIRQWakeup(IRQn());
    IRQClear();
    IRQEnable();
    // must not sleep while communicating
    PLATFORM_DEEP_SLEEP_DISABLE();
}

void I2C::TransactionCleanup()
{
    ASSERT(GETBIT(s_locks, Index()));
    RESBIT(s_locks, Index());

    IEN = 0;
    IRQDisable();
    PLATFORM_DEEP_SLEEP_ENABLE();
}

async(I2C::_Address, Operation op)
async_def()
{
    if (op.start)
    {
        // start a new transaction, acquire the lock first
        await_acquire(s_locks, BIT(Index()));
        TransactionInit();
    }

    // a transaction must have been started
    ASSERT(GETBIT(s_locks, Index()));
    auto state = State();
    auto flags = ClearFlags();

    if (op.noAddress)
    {
        // just verify the state
        if (state != BusAddrAck && state != BusDataAck)
        {
            DBGERR("Bus not after an ACK, cannot continue");
            async_return(false);
        }
        async_return(true);
    }
    else if (op.start)
    {
        // wait for bus to become idle from previous operation
        IEN = AwaitFlags;
        if (!await_mask_ms(STATE, _I2C_STATE_STATE_MASK, I2C_STATE_STATE_IDLE, I2C_TIMEOUT))
        {
            DBGERR("Bus not idle, resetting");
            await(Reset);
        }

        TxClearBuffer();
        RxClearBuffer();
        Abort();
        flags = ClearFlags();

        DIAG(DIAG_TRANS, "START >> %c %02X", op.read ? 'R' : 'W', op.address);
        Send(op.fullAddress);
        Start();
    }
    else
    {
        if (state != BusDataAck)
        {
            DBGERR("Bus not after data ACK, cannot restart");
            async_return(false);
        }

        DIAG(DIAG_TRANS, "REP-START >> %c %02X", op.read ? 'R' : 'W', op.address);
        Start();
        Send(op.fullAddress);
    }

    for (;;)
    {
        bool timeout = !await_mask_not_ms(IF, PrepWait(AwaitFlags), 0, I2C_TIMEOUT);
        flags = ClearFlags();

        if (timeout)
        {
            DBGERR("timeout waiting for address ACK");
            async_return(false);
        }

        if (HandleError(flags))
        {
            // address not ACK-ed
            DBGERR("error while waiting for address ACK");
            async_return(false);
        }
        else if (flags.nack)
        {
            // address NAK == device not present, do not log an error
            async_return(false);
        }
        else if (flags.ack)
        {
            async_return(true);
        }
        else
        {
            await(OnUnhandledErrorAsync, flags);
        }
    }
}
async_end

async(I2C::_Read, Operation op, char* data)
async_def(uint32_t wx)
{
    if (!await(_Address, op))
    {
        goto fail;
    }

    for (f.wx = 0; f.wx < op.length; f.wx++)
    {
        bool timeout = !await_mask_not_ms(IF, PrepWait(AwaitFlagsNoAck), 0, I2C_TIMEOUT);	// don't care about ACK or NAK
        auto flags = ClearFlags();

        if (timeout)
        {
            DBGERR("timeout waiting for RX data");
            await(Reset);
            break;
        }

        if (HandleError(flags))
        {
            DBGERR("error waiting for RX data");
            break;
        }
        else if (flags.dataValid)
        {
            data[f.wx] = Receive();
            if (f.wx + 1 == op.length)
            {
                DIAG(DIAG_READ | DIAG_ACK, "%02X >NAK", data[f.wx]);
                NACK();
            }
            else
            {
                DIAG(DIAG_READ | DIAG_ACK, "%02X >ACK", data[f.wx]);
                ACK();
            }
        }
        else
        {
            await(OnUnhandledErrorAsync, flags);
        }
    }

    if (f.wx != op.length || op.stop)
    {
fail:
        DIAG(DIAG_TRANS, "STOP");
        Stop();
        TransactionCleanup();
    }

    async_return(f.wx);
}
async_end

async(I2C::_Write, Operation op, const char* data)
async_def(uint32_t wx)
{
    if (!await(_Address, op))
    {
        goto fail;
    }

    for (f.wx = 0; f.wx < op.length; f.wx++)
    {
        DIAG(DIAG_WRITE | DIAG_DATA, ">> %02X (Data %d)", data[f.wx], f.wx);
        Send(data[f.wx]);

        for (;;)
        {
            bool timeout = !await_mask_not_ms(IF, PrepWait(AwaitFlags), 0, I2C_TIMEOUT);
            auto flags = ClearFlags();

            if (timeout)
            {
                DBGERR("timeout waiting for write data ACK");
                await(Reset);
                goto fail;
            }

            if (HandleError(flags))
            {
                DBGERR("error waiting for write data ACK");
                goto fail;
            }
            else if (flags.nack)
            {
                DIAG(DIAG_WRITE | DIAG_ACK, "<NAK");
                goto success;
            }
            else if (flags.ack)
            {
                DIAG(DIAG_WRITE | DIAG_ACK, "<ACK");
                break;
            }
            else
            {
                await(OnUnhandledErrorAsync, flags);
            }
        }
    }

success:
    if (f.wx != op.length || op.stop)
    {
fail:
        if (BusHeld())
        {
            DIAG(DIAG_TRANS, "STOP");
            Stop();
        }
        TransactionCleanup();
    }

    DIAG(DIAG_TRANS, "DONE");
    async_return(f.wx);
}
async_end

bool I2C::HandleError(StateFlags flags)
{
    if (flags.arbitrationLost || flags.busError || flags.masterStop)
    {
        return true;
    }

    if (flags.txOverflow)
    {
        DBGERR("tx overflow???");
        return true;
    }

    if (flags.clockLowTimeout)
    {
        DBGERR("clock low timeout");
        Abort();
        return true;
    }

    if (flags.busIdleTimeout)
    {
        DBGERR("bus idle timeout");
        return true;
    }

    if (flags.clockError)
    {
        DBGERR("clock error");
        return true;
    }
    return false;
}

bool I2C::HandleSlaveArbLost(StateFlags flags)
{
    if (flags.arbitrationLost)
    {
        DBGERR("slave arbitration lost - resetting to avoid SCL hang (see errata)");
        Abort();
        return true;
    }

    return false;
}

async(I2C::OnUnhandledErrorAsync, StateFlags flags)
async_def()
{
    DBGERR("Unhandled error");
    ASSERT(0);
    await(Reset);
}
async_end



async(I2C::SlaveWait, SlaveRequest request, Timeout timeout)
async_def(
    Timeout timeout;
    uint32_t expect, mask;
)
{
    ASSERT(IEN == 0);
    ASSERT(request != SlaveRequest::None);
    f.timeout = timeout.MakeAbsolute();

    if (await_acquire_timeout(s_locks, BIT(Index()), f.timeout))
    {
        Cortex_SetIRQWakeup(IRQn());
        IRQClear();
        IRQEnable();
        // no need to disable deep sleep yet, interrupt will wake us

        for (;;)
        {
            // wait for the right bus state
            if (!await_mask_not_timeout(IF, PrepWait(I2C_IF_ADDR | I2C_IF_ARBLOST), 0, timeout))
            {
                break;
            }

            if (HandleSlaveArbLost(ClearFlags()))
            {
                continue;
            }

            if (State() == BusState::BusAddr && BusBusy() && BusHeld())
            {
                auto addr = Receive();
                DIAG(DIAG_SLAVE, "SLAVE << %c %02X", GETBIT(addr, 0) ? 'R' : 'W', addr);
                if (request == SlaveRequest::Any || IsTransmitter() == (request == SlaveRequest::Read))
                {
                    // must not sleep while communicating
                    PLATFORM_DEEP_SLEEP_DISABLE();
                    async_return(int(GETBIT(addr, 0) ? SlaveRequest::Read : SlaveRequest::Write));
                }

                NACK();
            }
        }

        RESBIT(s_locks, Index());
        IEN = 0;
        IRQDisable();
    }

    async_return(int(SlaveRequest::None));
}
async_end

async(I2C::_SlaveWrite, const char* data, size_t length)
async_def(uint32_t wx)
{
    ASSERT(length);
    if (State() == BusState::BusAddr)
    {
        // we must acknowledge the address
        ACK();
    }

    for (f.wx = 0; f.wx < length; f.wx++)
    {
        DIAG(DIAG_WRITE | DIAG_DATA, ">> %02X (Data %d)", data[f.wx], f.wx);
        Send(data[f.wx]);

        for (;;)
        {
            bool timeout = !await_mask_not_ms(IF, PrepWait(AwaitFlagsSlaveWrite), 0, I2C_TIMEOUT);
            auto flags = ClearFlags();

            if (timeout)
            {
                DBGERR("timeout waiting for write data ACK");
                goto done;
            }

            if (HandleSlaveArbLost(flags))
            {
                goto done;
            }
            if (HandleError(flags))
            {
                DBGERR("error waiting for write data ACK");
                goto done;
            }
            else if (flags.nack)
            {
                DIAG(DIAG_WRITE | DIAG_ACK, "<NAK");
                goto done;
            }
            else if (flags.ack)
            {
                DIAG(DIAG_WRITE | DIAG_ACK, "<ACK");
                break;
            }
        }
    }

done:
    DIAG(DIAG_TRANS, "DONE");
    async_return(f.wx);
}
async_end

async(I2C::SlaveDone)
async_def()
{
    // slave must send something until master terminates the transaction
    while (SlaveActive() && IsTransmitter())
    {
        DIAG(DIAG_SLAVE, ">> 00 (DUMMY)");
        Send(0);
        await_mask_not_ms(IF, PrepWait(AwaitFlagsSlaveWrite), 0, I2C_TIMEOUT);
        HandleSlaveArbLost(ClearFlags());
    }

    TransactionCleanup();
}
async_end
