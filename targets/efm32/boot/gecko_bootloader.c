/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/boot/gecko_bootloader.c
 *
 * compiles required gecko bootloader components
 */

#include <boot/gecko_bootloader.h>

#define HardFault_Handler   gbl_HardFault_Handler

#define __rom_end__ __data_load_end

__attribute__((used, section(".bootloader.table"))) const MainBootloaderTable_t mainStageTable;

#include <core/btl_core.c>
#include <core/btl_main.c>

#include <plugin/parser/btl_image_parser.c>
#include <plugin/parser/ebl/btl_gbl_custom_tags.c>

#include <plugin/security/btl_crc32.c>
#include <plugin/security/btl_security_aes.c>
#include <plugin/security/btl_security_ecdsa.c>
#include <plugin/security/btl_security_sha256.c>
#include <plugin/security/btl_security_tokens.c>
#include <plugin/security/ecc/ecc.c>

#include <plugin/storage/btl_storage.c>

#if GECKO_BOOTLOADER_STORAGE_TYPE == INTERNAL_FLASH

#include <plugin/storage/internal_flash/btl_storage_internal_flash.c>

#endif
