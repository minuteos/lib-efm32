/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/boot/gecko_bootloader_config.h
 */

#pragma once

#include <gecko_bootloader_config.h>

#ifdef __cplusplus
#include <gecko_bootloader_callbacks.h>
#endif

#ifndef GECKO_BOOTLOADER_ENFORCE_SECURE_BOOT
#define GECKO_BOOTLOADER_ENFORCE_SECURE_BOOT    1
#endif

#ifndef GECKO_BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE
#define GECKO_BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE  0
#endif

#ifndef GECKO_BOOTLOADER_ENFORCE_SIGNED_UPGRADE
#define GECKO_BOOTLOADER_ENFORCE_SIGNED_UPGRADE     GECKO_BOOTLOADER_ENFORCE_SECURE_BOOT
#endif

#ifndef GECKO_BOOTLOADER_APP_SPACE
#define GECKO_BOOTLOADER_APP_SPACE              (FLASH_SIZE / 2)
#endif

#ifndef GECKO_BOOTLOADER_STORAGE_NUM_SLOTS
#define GECKO_BOOTLOADER_STORAGE_NUM_SLOTS      1
#endif

#ifndef GECKO_BOOTLOADER_STORAGE_SLOTS
// use the entire application area of FLASH for storage by default
#ifndef GECKO_BOOTLOADER_STORAGE_RESERVE
#define GECKO_BOOTLOADER_STORAGE_RESERVE        0
#endif

#define GECKO_BOOTLOADER_STORAGE_SLOTS  { { GECKO_BOOTLOADER_STORAGE_RESERVE, GECKO_BOOTLOADER_APP_SPACE - GECKO_BOOTLOADER_STORAGE_RESERVE } }
#endif
