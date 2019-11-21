#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32pg1b/Include.mk
#

TARGETS += efm32-series1 cortex-m4f

EFM32_DEVICE ?= EFM32PG1B
# use the most full-featured part by default
EFM32_PART ?= efm32pg1b200f256gm48.h
GBL_VARIANT = efx32jg1b
