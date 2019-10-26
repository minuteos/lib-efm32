ENTRY(FirstStage)

SECTIONS {
    .firststage :
    {
        KEEP(*(.firststage))
        KEEP(*(.firststage*))
        . = ALIGN(2048);
    } >FLASH_BOOT
}
