/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/io/USARTRxPipe.cpp
 */

#include "USARTRxPipe.h"

#include <hw/LDMA.h>

#define TRACE_DMA       1
#define TRACE_PIPE      2
#define TRACE_DATA      4

//#define USART_RXPIPE_TRACE   TRACE_DATA

#define MYDBG(fmt, ...)      DBGL("USART%d_RX: " fmt, usart.Index(), ## __VA_ARGS__)

#if USART_RXPIPE_TRACE
#define MYTRACE(flag, ...) if ((USART_RXPIPE_TRACE) & (flag)) { MYDBG(__VA_ARGS__); }
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
async_def()
{
    MYDBG("Starting");

    dmapos = pipe.Position();
    PLATFORM_DEEP_SLEEP_DISABLE();

    while (pipe.AvailableAfter(dmapos) || await(pipe.Allocate, blockSize))
    {
        bool running = dma.IsEnabled();
        if (running)
        {
            dma.Disable();
            while (dma.IsBusy());   // make sure DMA is stopped before linking new segments
        }

        // either start or link the next segment
        auto buf = pipe.GetBufferAt(dmapos).Left(LDMADescriptor::MaximumTransferSize);
        LDMADescriptor* desc;
        if (!running)
        {
            // start new DMA
            MYTRACE(TRACE_DMA, "BUF =%p+%d", buf.Pointer(), buf.Length());
            // we can remove the volatile qualifier, DMA is stopped
            desc = (LDMADescriptor*)&dma.RootDescriptor();
        }
        else
        {
            // link another segment
            desc = MemPoolAlloc<LDMADescriptor>();
            if (!desc)
            {
                // cannot allocate descriptor, need to wait
                auto mon = MemPoolGet<LDMADescriptor>()->WatchPointer();
                if (running)
                {
                    dma.Enable();
                }
                await_mask_not(*mon, ~0u, *mon);
                continue;
            }
            LDMADescriptor* d = (LDMADescriptor*)&dma.RootDescriptor(); // we can remove the volatile qualifier, DMA is stopped
            if (auto firstLink = d->LinkedDescriptor())
            {
                // find the end of the list, we also must have an allocList at this point
                ASSERT(allocList);
                d = firstLink;
                while (auto next = d->LinkedDescriptor())
                {
                    d = next;
                }
            }
            else
            {
                // linking first descriptor - it is possible that allocList still exists, in this case
                // we need to link the descriptor at the end of it anyway, to avoid breaking the chain
                if (auto al = allocList)
                {
                    while (auto next = al->LinkedDescriptor())
                        al = next;
                    al->Link(desc);
                }
                else
                {
                    allocList = desc;
                }
            }
            MYTRACE(TRACE_DMA, "LNK+%p %p+%d (%p)", desc, buf.Pointer(), buf.Length(), d);
            d->Link(desc);
        }
        desc->SetTransfer(&usart.RXDATA, buf.Pointer(), buf.Length(), LDMADescriptor::P2M | LDMADescriptor::UnitByte);
        dmapos += buf.Length();
        dma.Enable();
        usart.RxEnable();
        StartDMAMonitor();
    }

    // release channel
    usart.RxDisable();
    dma.SourceNone();
    dma.RootDescriptor().Destination(NULL); // this wakes up the rx task
    FreeUnusedDescriptors(NULL);
    await_signal_off(dmaMonitor);
    PLATFORM_DEEP_SLEEP_ENABLE();
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

void USARTRxPipe::FreeUnusedDescriptors(LDMADescriptor* stop)
{
    auto p = allocList;
    while (p && p != stop)
    {
        auto next = p->LinkedDescriptor();
        MYTRACE(TRACE_DMA, "LNK-%p %p+%d", p, p->Destination(), p->Count());
        MemPoolFree(p);
        p = next;
    }
    allocList = p;
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
        // we can free all descriptors up to the currently linked one
        FreeUnusedDescriptors(dma.LinkedDescriptor());
        auto& dstPtr = dma.RootDescriptor().DST;
        auto pMax = (char*)dstPtr;
        NVIC_ClearPendingIRQ(usart.RxIRQn());
        if (!pMax)
        {
            // DST gets zeroed explicitly when stopping
            break;
        }
        size_t count;
        Buffer buf;
        while (pipe.Available() && (buf = pipe.GetBuffer(), count = pMax - buf.Pointer()))
        {
            // if the DMA is already writing to another buffer, count will be bigger than buffer size regardless of
            // where the other buffer is located due to overflow
            auto newData = buf.Left(count);
            MYTRACE(TRACE_PIPE, "%p+%d=%p", newData.Pointer(), newData.Length(), newData.end());
            MYTRACE(TRACE_DATA, "[%p] << %H", newData.Pointer(), newData);
            pipe.Advance(newData.Length());
            if (count <= buf.Length())
            {
                break;
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
