/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/boot/firststage.cpp
 *
 * First stage bootloader for EFM32 series MCUs
 * (stub implementation, simply jumps to the main bootloader)
 */

#include <base/base.h>

#include <btl_interface.h>

BEGIN_EXTERN_C

__attribute__((naked, section(".firststage")))
void FirstStage()
{
    extern uint32_t __boot_start;

    // we start without a stack pointer, so let's fix that first
    SCB->VTOR = (uint32_t)&__boot_start;
    __asm volatile("ldr sp, [%0]\nldr pc, [%0, #4]" : : "r"(&__boot_start));
}

BareBootTable_t __boot_start;

__attribute__((used, section(".firststage.table")))
const FirstBootloaderTable_t _boot_table_first = {
    { BOOTLOADER_MAGIC_FIRST_STAGE, 0, 0 },
    &__boot_start,
    NULL,
};

END_EXTERN_C
