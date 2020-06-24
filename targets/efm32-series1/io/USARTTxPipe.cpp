/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/io/USARTTxPipe.cpp
 */

#include "USARTTxPipe.h"

//#define USART_TXPIPE_TRACE    1

#define MYDBG(fmt, ...)    DBGL("USART%d_TX: " fmt, usart.Index(), ## __VA_ARGS__)

#if USART_TXPIPE_TRACE
#define MYTRACE MYDBG
#else
#define MYTRACE(...)
#endif

namespace io
{

async(USARTTxPipe::Start)
async_def_sync()
{
    if (!running)
    {
        running = true;
        kernel::Task::Run(this, &USARTTxPipe::Task);
    }
    async_return(true);
}
async_end

async(USARTTxPipe::Stop, Timeout timeout)
async_def()
{
    async_return(await_signal_off_timeout(running, timeout));
}
async_end

async(USARTTxPipe::Task)
async_def(
    LDMAChannelHandle dma;
    LDMADescriptor desc;
)
{
    f.dma = LDMA->GetUSARTChannel(usart.Index(), LDMAChannel::USARTSignal::TxFree, true);
    MYDBG("Starting");

    while (await(pipe.Require))
    {
        auto span = pipe.GetSpan();
        MYTRACE(">> %H", span);
        f.desc.SetTransfer(span.Pointer(), &usart.TXDATA, span.Length(), LDMADescriptor::M2P | LDMADescriptor::UnitByte | LDMADescriptor::SetDone);
        f.dma.LinkLoad(f.desc);
        usart.TxEnable();
        await(f.dma.WaitForDoneFlag);
        MYTRACE(">> DONE");
        pipe.Advance(f.desc.Count());
    }

    MYDBG("Finished");
    usart.TxDisable();
    f.dma.SourceNone();
    running = false;
}
async_end

}
