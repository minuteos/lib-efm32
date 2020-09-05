/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/cmsis.h
 *
 * Includes the appropriate CMSIS headers for the target device
 */

#pragma once

#define _EFM32_PART_HEADER  <EFM32_PART_HEADER>
#include _EFM32_PART_HEADER

// define atomic bit modification helpers

#ifdef PER_BITSET_MEM_BASE

#define EFM32_PERIPHERAL_BITMOD

#ifdef __cplusplus
template<typename T> ALWAYS_INLINE volatile T* EFM32_BITMODPTR(bool set, volatile T* ptr) { return (volatile T*)((intptr_t)ptr | (set ? PER_BITSET_MEM_BASE : PER_BITCLR_MEM_BASE)); }
#endif

#define EFM32_BITMOD(set, reg, mask)	(EFM32_BITMODPTR(set, &(reg))[0] = (mask))
#define EFM32_BITCLR(reg, mask)	EFM32_BITMOD(false,reg,mask)
#define EFM32_BITSET(reg, mask)	EFM32_BITMOD(true,reg,mask)
#define EFM32_BITSET_REG(reg, mask)  EFM32_BITSET(reg, mask)
#define EFM32_BITCLR_REG(reg, mask)  EFM32_BITCLR(reg, mask)

#define EFM32_IFS(per)  (per)->IFS
#define EFM32_IFC(per)  (per)->IFC
#define EFM32_IFC_READ(per)  (per)->IFC

#elif _SILICON_LABS_32B_SERIES >= 2

#define EFM32_PERIPHERAL_BITTGL
#define EFM32_PERIPHERAL_BITMOD

#ifdef __cplusplus
template<typename T> ALWAYS_INLINE volatile T* EFM32_BITMODPTR(bool set, volatile T* ptr) { return (volatile T*)((intptr_t)ptr | (set ? 0x1000 : 0x2000)); }
template<typename T> ALWAYS_INLINE volatile T* EFM32_BITTGLPTR(volatile T* ptr) { return (volatile T*)((intptr_t)ptr | 0x3000); }
#endif

#define EFM32_BITMOD(set, reg, mask)	(EFM32_BITMODPTR(set, &(reg))[0] = (mask))
#define EFM32_BITCLR(reg, mask)	EFM32_BITMOD(false,reg,mask)
#define EFM32_BITSET(reg, mask)	EFM32_BITMOD(true,reg,mask)
#define EFM32_BITTGL(reg, mask)	(EFM32_BITTGLPTR(&(reg))[0] = (mask))
#define EFM32_BITCLR_REG(reg, mask)	((reg ## _CLR) = (mask))
#define EFM32_BITSET_REG(reg, mask)	((reg ## _SET) = (mask))
#define EFM32_BITTGL_REG(reg, mask)	((reg ## _TGL) = (mask))

#define EFM32_IFS(per)  (per)->IF_SET
#define EFM32_IFC(per)  (per)->IF_CLR
#define EFM32_IFC_READ(per)  ((per)->IF_CLR = (per)->IF)

#else

#define EFM32_BITMOD(set, reg, mask)	((set) ? EFM32_BITSET(reg, mask): EFM32_BITCLR(reg, mask))
#define EFM32_BITSET(reg, mask)	(*(volatile uint32_t*)&(reg) |= mask)
#define EFM32_BITCLR(reg, mask)	(*(volatile uint32_t*)&(reg) &= ~mask)

#endif
