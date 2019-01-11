/*
 * Memory map for the EFM32GG11BxxxF2048
 */
 
MEMORY {
	FLASH (rx) : ORIGIN = 0, LENGTH = 2048K,
	RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 512K
}
