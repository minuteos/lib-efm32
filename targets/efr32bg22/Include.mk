#
# Copyright (c) 2020 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efr32bg22/Include.mk
#

TARGETS += efr32-series2

EFM32_DEVICE ?= EFR32BG22
# use the most full-featured part by default
EFM32_PART ?= efr32bg22c224f512gm32.h
GBL_VARIANT = efr32xg22
RAIL_VARIANT = efr32xg22
JLINK_DEVICE ?= EFR32BG22CxxxF512
