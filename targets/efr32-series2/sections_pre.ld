/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series2/sections_pre.ld
 *
 * Inserts bootloader at the beginning of flash
 */

SECTIONS {
    .bin.bootloader : {
        KEEP(*(.binboot*))
        . = ALIGN(8192);
    } >FLASH
}
