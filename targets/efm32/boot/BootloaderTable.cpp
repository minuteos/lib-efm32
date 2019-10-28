/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/boot/BootloaderTable.cpp
 *
 * Gecko Bootloader table
 */

#include <base/base.h>

#include <btl_interface.h>

#include <ld_symbols.h>

#include "GblParser.h"

BEGIN_EXTERN_C

extern char __boot_start;

int32_t _boot_nop()
{
    return BOOTLOADER_OK;
}

__attribute__((used, section(".bootloader.table")))
const MainBootloaderTable_t _boot_table_main =
{
    .header = { BOOTLOADER_MAGIC_MAIN, BOOTLOADER_HEADER_VERSION_MAIN, 0 /* TODO: version */ },
    .size = (uint32_t)(&__data_load_end) - BTL_MAIN_STAGE_BASE + 4,
    .startOfAppSpace = (BareBootTable_t*)BTL_APPLICATION_BASE,
    .endOfAppSpace = (uint32_t*)(FLASH_BASE + FLASH_SIZE - BTL_APPLICATION_BASE),
    .capabilities = 0,
    .init = &_boot_nop,
    .deinit = &_boot_nop,
    .verifyApplication = &GblParser::Verify,
    .initParser = &GblParser::Init,
    .parseBuffer = &GblParser::Parse,
};

END_EXTERN_C
