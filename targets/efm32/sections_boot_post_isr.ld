/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/sections_boot_post_isr.ld
 *
 * Additional section inserted after the bootloader ISR table, which adds
 * the Gecko Bootloader table (and pointer to it) at the appropriate places
 */

SECTIONS {
    .text.boot_table : {
        . = __boot_start + 0x28;   /* pointer to main bootloader table */
        LONG(__boot_header);
        . = __boot_start + 0x34;   /* pointer to bootloader application properties */
        LONG(__boot_props);
        __boot_header = .;
        KEEP(*(.bootloader.table));
        __boot_props = .;
        KEEP(*(.bootloader.props));
    } >FLASH_BOOT
}
