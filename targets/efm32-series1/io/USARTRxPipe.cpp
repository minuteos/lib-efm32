/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/io/USARTRxPipe.cpp
 */

#include "USARTRxPipe.h"

#include <hw/LDMA.h>

//#define USART_RXPIPE_TRACE    1

#define MYDBG(fmt, ...)      DBGL("USART%d_RX: " fmt, usart.Index(), ## __VA_ARGS__)

#if USART_RXPIPE_TRACE
#define MYTRACE MYDBG
#else
#define MYTRACE(...)
#endif

namespace io
{

async(USARTRxPipe::Start)
async_def()
{
    if (!dma.IsValid())
    {
        dma = LDMA->GetUSARTChannel(usart.Index(), LDMAChannel::USARTSignal::RxDataValid);
        kernel::Task::Run(this, &USARTRxPipe::DMATask);
    }
    await_signal(dmaMonitor);
}
async_end

async(USARTRxPipe::Stop, Timeout timeout)
async_def(
    Timeout timeout;
)
{
    pipe.Close();
    if (!dma.IsValid())
    {
        async_return(true);
    }

    MYDBG("Stopping");
    dma.Disable();
    dma.SetDone();
    if (!await_mask_timeout(dma, ~0u, ~0u, timeout))
    {
        MYDBG("Failed to stop");
        async_return(false);
    }
    else
    {
        async_return(true);
    }
}
async_end

async(USARTRxPipe::DMATask)
async_def(
    LDMADescriptor desc;
    bool first;
)
{
    f.first = true;
    MYDBG("Starting");

    dmapos = pipe.Position();

    while (pipe.AvailableAfter(dmapos) || await(pipe.Allocate, blockSize))
    {
        dma.Disable();
        // either start or link the next segment
        auto buf = pipe.GetBufferAt(dmapos);
        f.desc.SetTransfer(&usart.RXDATA, buf.Pointer(), buf.Length(), LDMADescriptor::P2M | LDMADescriptor::UnitByte | LDMADescriptor::SetDone);
        dmapos += buf.Length();
        if (f.first || dma.IsDone())
        {
            // start new DMA
            MYTRACE("Initial RX buffer %d bytes @ %p", buf.Length(), buf.Pointer());
            f.first = false;
            dma.LinkLoad(f.desc);
            ASSERT(dma.IsEnabled());
            usart.RxEnable();
            StartDMAMonitor();
        }
        else
        {
            // link to current DMA descriptor
            MYTRACE("Linked RX buffer %d bytes @ %p", buf.Length(), buf.Pointer());
            dma.RootDescriptor().Link(&f.desc);
            dma.ClearDone();
            dma.Enable();
            StartDMAMonitor();
            // wait for the previous block to complete before enqueuing another one
            await(dma.WaitForDoneFlag);
            MYTRACE("Initial buffer finished");
        }
    }

    // release channel
    usart.RxDisable();
    dma.SourceNone();
    dma.RootDescriptor().Destination(NULL); // this wakes up the rx task
    await_signal_off(dmaMonitor);
    dma = LDMAChannelHandle();
    MYDBG("Finished");
}
async_end

void USARTRxPipe::StartDMAMonitor()
{
    if (!dmaMonitor)
    {
        dmaMonitor = true;
        kernel::Task::Run(this, &USARTRxPipe::DMAMonitor);
    }
}

async(USARTRxPipe::DMAMonitor)
async_def()
{
    MYDBG("Monitor starting");
    usart.IEN |= USART_IEN_RXDATAV;
    Cortex_SetIRQWakeup(usart.RxIRQn());
    NVIC_EnableIRQ(usart.RxIRQn());

    while (!pipe.IsClosed())
    {
        auto& dstPtr = dma.RootDescriptor().DST;
        auto pMax = (char*)dstPtr;
        NVIC_ClearPendingIRQ(usart.RxIRQn());
        MYTRACE("pMax = %p", pMax);
        if (!pMax)
        {
            // DST gets zeroed explicitly when stopping
            break;
        }
        auto buf = pipe.GetBuffer();
        while (size_t count = pMax - buf.Pointer())
        {
            // if the DMA is already writing to another buffer, count will be bigger than buffer size regardless of
            // where the other buffer is located due to overflow
            MYTRACE("<< %H", buf.Left(count));
            pipe.Advance(std::min(count, buf.Length()));
            if (count <= buf.Length())
            {
                break;
            }
            else
            {
                buf = pipe.GetBuffer();
            }
        }
        await_mask_not(dstPtr, ~0u, pMax);
    }

    MYDBG("Monitor finished");
    NVIC_DisableIRQ(usart.RxIRQn());
    usart.IEN &= ~USART_IEN_RXDATAV;
    dmaMonitor = false;
}
async_end

}
