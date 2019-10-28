/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/boot/GblParser.cpp
 *
 * GBL file parser and application verifier
 * (stub implementation, does nothing, just allows the binapploader to work)
 */

#include "GblParser.h"

int32_t GblParser::Init(BootloaderParserContext_t* context, size_t contextSize)
{
    return BOOTLOADER_OK;
}

 int32_t GblParser::Parse(BootloaderParserContext_t* context, const BootloaderParserCallbacks_t* callbacks, uint8_t* data, size_t length)
 {
     return BOOTLOADER_OK;
 }

bool GblParser::Verify(uint32_t startOfApp)
{
    return true;
}
