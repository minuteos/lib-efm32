/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/sections_post.ld
 *
 * Includes the binary bootloader image in the primary output
 */

SECTIONS {
    .bin.bootloader : {
        KEEP(*(.binboot*))
    } >FLASH_BOOT
}
