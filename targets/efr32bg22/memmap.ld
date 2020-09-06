/*
 * Memory map for the EFR32BG22CxxxF512
 */

MEMORY {
    FLASH (rx) : ORIGIN = 0, LENGTH = 512K,
    FLASH_BOOT (rx) : ORIGIN = 0, LENGTH = 32K,
    RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 32K
}
