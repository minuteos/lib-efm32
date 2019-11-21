/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/sections_boot_pre.ld
 *
 * Inserts the first stage bootloader before the main bootloader
 */

SECTIONS {
    .bin.firststage : {
        KEEP(*(.bin.firststage))
        . = ALIGN(2048);
    } >FLASH_BOOT
}
