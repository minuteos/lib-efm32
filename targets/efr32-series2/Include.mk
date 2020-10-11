#
# Copyright (c) 2020 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efr32-series2/Include.mk
#
# Makefile additions for all SiLabs EFR32 Series 2 MCUs
#

TARGETS += efr32 efm32-series2 cortex-m33
EFM32_RAIL_CHIP = efr32xg2x

ifdef BOOTLOADER_BUILD
# bootloader support libs are built with softfp
CORTEX_FLOAT_ABI = softfp
endif
