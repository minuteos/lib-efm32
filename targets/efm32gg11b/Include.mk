#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32gg11b/Include.mk
#

TARGETS += efm32-series1 efm32-usb cortex-m4f

EFM32_DEVICE ?= EFM32GG11B
# use the most full-featured part by default
EFM32_PART ?= efm32gg11b820*
GBL_VARIANT = efx32gg11b
JLINK_DEVICE ?= EFM32GG11B820F2048
