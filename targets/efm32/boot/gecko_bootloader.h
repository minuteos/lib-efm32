/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/boot/gecko_bootloader.h
 *
 * prepares environment for and includes gecko all required Gecko Bootloader headers
 */

#include <base/base.h>

BEGIN_EXTERN_C

#define BTL_CONFIG_FILE <boot/gecko_bootloader_config.h>
#define BOOTLOADER_SUPPORT_STORAGE      1

#define BTL_STORAGE_BOOTLOAD_LIST_LENGTH    1
#define BTL_PLUGIN_STORAGE_NUM_SLOTS    GECKO_BOOTLOADER_STORAGE_NUM_SLOTS
#define BTL_PLUGIN_STORAGE_SLOTS        GECKO_BOOTLOADER_STORAGE_SLOTS
#define BTL_APP_SPACE_SIZE              GECKO_BOOTLOADER_APP_SPACE
#define LIBRARY_BUILD

#ifdef TRACE
#define BTL_PLUGIN_DEBUG_PRINT  1

ALWAYS_INLINE void btl_debugInit(void) { }
ALWAYS_INLINE void btl_debugWriteChar(char c) { _DBGCHAR(c); }
ALWAYS_INLINE void btl_debugWriteString(const char * s) { _DBG("%s", s); }
ALWAYS_INLINE void btl_debugWriteLine(const char * s) { _DBG("%s\n", s); }

ALWAYS_INLINE void btl_debugWriteCharHex(uint8_t number) { _DBG("%02X", number); }
ALWAYS_INLINE void btl_debugWriteShortHex(uint16_t number) { _DBG("%04X", number); }
ALWAYS_INLINE void btl_debugWriteWordHex(uint32_t number) { _DBG("%08X", number); }
ALWAYS_INLINE void btl_debugWriteInt(int number) { _DBG("%d", number); }
ALWAYS_INLINE void btl_debugWriteNewline(void) { _DBGCHAR('\n'); }

#endif

#include <config/btl_config.h>

#if GECKO_BOOTLOADER_ENFORCE_SIGNED_UPGRADE
#define BOOTLOADER_ENFORCE_SIGNED_UPGRADE    1
#endif

#if GECKO_BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE
#define BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE    1
#endif

#if GECKO_BOOTLOADER_ENFORCE_SECURE_BOOT
#define BOOTLOADER_ENFORCE_SECURE_BOOT 1
#endif

#include <api/btl_interface.h>
#include <core/btl_bootload.h>
#include <core/btl_parse.h>

#include <plugin/storage/btl_storage.h>

extern const BootloaderStorageFunctions_t storageFunctions;

END_EXTERN_C
