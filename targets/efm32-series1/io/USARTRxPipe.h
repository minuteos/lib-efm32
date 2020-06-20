/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/io/USARTRxPipe.h
 */

#pragma once

#include <io/io.h>

#include <hw/LDMA.h>
#include <hw/USART.h>

namespace io
{

class USARTRxPipe
{
public:
    USARTRxPipe(USART& usart, PipeWriter pipe, size_t blockSize = 256)
        : usart(usart), pipe(pipe), blockSize(blockSize)
    {
    }

    USART& GetUSART() const { return usart; }

    async(Start);
    async(Stop, Timeout timeout = Timeout::Infinite);

private:
    USART& usart;
    PipeWriter pipe;
    size_t blockSize;
    LDMAChannelHandle dma;
    PipePosition dmapos;
    bool dmaMonitor = false;

    LDMADescriptor* freeDescriptors;

    async(DMATask);
    async(DMAMonitor);

    void StartDMAMonitor();
};

}
