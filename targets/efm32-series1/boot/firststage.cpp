/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/boot/firststage.cpp
 *
 * First stage bootloader for EFx32xG1x series MCUs
 */

#include <base/base.h>

BEGIN_EXTERN_C

__attribute__((naked, section(".firststage")))
void FirstStage()
{
    extern uint32_t __boot_start;

    // we start without a stack pointer, so let's fix that first
    SCB->VTOR = (uint32_t)&__boot_start;
    __asm volatile("ldr sp, [%0]\nldr pc, [%0, #4]" : : "r"(&__boot_start));
}

END_EXTERN_C
