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

#ifdef __cplusplus
template<typename T> ALWAYS_INLINE volatile T* EFM32_BITMODPTR(bool set, volatile T* ptr) { return (volatile T*)((intptr_t)ptr | (set ? PER_BITSET_MEM_BASE : PER_BITCLR_MEM_BASE)); }
#endif

#define EFM32_BITMOD(set, reg, mask)	(EFM32_BITMODPTR(set, &(reg))[0] = (mask))
#define EFM32_BITCLR(reg, mask)	EFM32_BITMOD(false,reg,mask)
#define EFM32_BITSET(reg, mask)	EFM32_BITMOD(true,reg,mask)

#else

#define EFM32_BITMOD(set, reg, mask)	((set) ? EFM32_BITSET(reg, mask): EFM32_BITCLR(reg, mask))
#define EFM32_BITSET(reg, mask)	(*(volatile uint32_t*)&(reg) |= mask)
#define EFM32_BITCLR(reg, mask)	(*(volatile uint32_t*)&(reg) &= ~mask)

#endif
