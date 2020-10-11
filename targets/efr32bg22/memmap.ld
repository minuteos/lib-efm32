/*
 * Memory map for the EFR32BG22CxxxF512
 */

MEMORY {
    FLASH (rx) : ORIGIN = 0, LENGTH = 512K,
    RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 32K
}

/* Bootloader actually comes at the beginning of FLASH */
REGION_ALIAS("FLASH_BOOT", FLASH)

/* 32K is too little to waste 1/4 of it on stack */
STACK_RESERVED = 4K;
