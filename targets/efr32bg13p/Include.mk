#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efr32bg13p/Include.mk
#

TARGETS += efr32-series1x

EFM32_DEVICE ?= EFM32BG13P
# use the most full-featured part by default
EFM32_PART ?= efm32bg13p632f512gm.h
GBL_VARIANT = efx32xg13
JLINK_DEVICE ?= EFR32BG13PxxxF512
