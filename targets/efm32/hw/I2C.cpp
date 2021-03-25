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

//#define EFM32_I2C_DEBUG   1

#if EFM32_I2C_DEBUG
#define DIAG(...)	DBGCL(Index() ? "I2C1" : "I2C0", __VA_ARGS__)
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

    NVIC_SetPriority(IRQn(), 0xFF);
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

        DIAG("START >> %c %02X", op.read ? 'R' : 'W', op.address);
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

        DIAG("REP-START >> %c %02X", op.read ? 'R' : 'W', op.address);
        Start();
        Send(op.fullAddress);
    }

    for (;;)
    {
        bool timeout = !await_mask_not_ms(IF, IEN = AwaitFlags, 0, I2C_TIMEOUT);
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
        bool timeout = !await_mask_not_ms(IF, IEN = AwaitFlagsNoAck, 0, I2C_TIMEOUT);	// don't care about ACK or NAK
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
                DIAG("%02X >NAK", data[f.wx]);
                NACK();
            }
            else
            {
                DIAG("%02X >ACK", data[f.wx]);
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
        DIAG("STOP");
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
        DIAG(">> %02X (Data %d)", data[f.wx], f.wx);
        Send(data[f.wx]);

        for (;;)
        {
            bool timeout = !await_mask_not_ms(IF, IEN = AwaitFlags, 0, I2C_TIMEOUT);
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
                DIAG("<NAK");
                goto success;
            }
            else if (flags.ack)
            {
                DIAG("<ACK");
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
            DIAG("STOP");
            Stop();
        }
        TransactionCleanup();
    }

    DIAG("DONE");
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

async(I2C::OnUnhandledErrorAsync, StateFlags flags)
async_def()
{
    DBGERR("Unhandled error");
    ASSERT(0);
    await(Reset);
}
async_end
