/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/io/USARTTxPipe.h
 */

#pragma once

#include <io/io.h>

#include <hw/USART.h>

namespace io
{

class USARTTxPipe
{
public:
    USARTTxPipe(USART& usart, PipeReader pipe)
        : usart(usart), pipe(pipe)
    {
    }

    USART& GetUSART() const { return usart; }

    async(Start);
    async(Stop, Timeout timeout = Timeout::Infinite);

private:
    USART& usart;
    PipeReader pipe;
    bool running = false;

    LDMADescriptor* freeDescriptors;

    async(Task);
};

}
