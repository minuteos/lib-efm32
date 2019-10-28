/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/boot/GblParser.h
 */

#include <kernel/kernel.h>

#include <btl_interface.h>

class GblParser
{
public:
    static int32_t Init(BootloaderParserContext_t* context, size_t contextSize);
    static int32_t Parse(BootloaderParserContext_t* context, const BootloaderParserCallbacks_t* callbacks, uint8_t* data, size_t length);
    static bool Verify(uint32_t startOfApp);
};
