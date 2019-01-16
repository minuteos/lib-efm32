/*
 * Memory map for the EFM32PG1BxxxF256
 */
 
MEMORY {
	FLASH (rx) : ORIGIN = 0, LENGTH = 256K,
	RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 32K
}
