/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/sections_boot_pre.ld
 *
 * Inserts the first stage bootloader before the main bootloader
 */

ENTRY(FirstStage)

SECTIONS {
    .firststage : {
        __firststage_start = .;
        KEEP(*(.firststage))
        . = __firststage_start + 0x28;   /* pointer to table must be at offset 0x28 */
        LONG(__firststage_table);
        __firststage_table = .;
        KEEP(*(.firststage*))
        . = ALIGN(2048);
    } >FLASH_BOOT
}
