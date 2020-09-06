#
# Copyright (c) 2020 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32-series1x/boot/Include.mk
#

ifdef BOOTLOADER_BUILD

GBL_FIRST_STAGE = $(OBJDIR)first-stage.o
GBL_FIRST_STAGE_SRC = $(EFM32_SDK_BOOT)build/first_stage/gcc/first_stage_btl_$(GBL_VARIANT).s37

ADDITIONAL_BLOBS += $(GBL_FIRST_STAGE)

$(GBL_FIRST_STAGE_SRC): $(EFM32_SDK_BOOT)

$(GBL_FIRST_STAGE): $(GBL_FIRST_STAGE_SRC)
	$(OBJCOPY) -I srec -O elf32-littlearm -B arm --rename-section .sec1=.bin.firststage $< $@

endif
