/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efr32-apploader/sections_post_isr.ld
 *
 * Additional section inserted after the main ISR table, which adds
 * the Gecko Bootloader Application Properties table (and pointer to it)
 * at the appropriate places
 */

SECTIONS {
    .text.application_properties_ptr : {
        . = __app_start + 0x34;     /* pointer must be at offset 0x34 */
        LONG(__app_properties);
    } >FLASH =0xFF

    .text.application_properties : {
        . = __app_start + 0xFC;     /* properties themselves must be at 0xFC */
        __app_properties = .;
        KEEP(*(.application_properties));
        . = ALIGN(8);               /* do not leave a hole before the next section */
    } >FLASH =0xFF

    .text.gatt : {
        KEEP(*(.gatt_header));
        KEEP(*(.gatt_data));
        . = ALIGN(8);               /* do not leave a hole before main .text section */
    } > FLASH =0xFF
}
