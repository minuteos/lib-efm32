#
# Copyright (c) 2020 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efr32/rail/IncludePost.mk
#

RAIL_LIB ?= rail_$(RAIL_VARIANT)_gcc_release

ifndef BOOTLOADER_BUILD
LIBS += $(RAIL_LIB)
endif
