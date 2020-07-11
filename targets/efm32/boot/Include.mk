#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/boot/Include.mk
#

ifneq (bootloader,$(NAME))

COMPONENTS += build-gbl

else

TARGETS += gecko-bootloader

GBL_FIRST_STAGE = $(OBJDIR)first-stage.o
GBL_FIRST_STAGE_SRC = $(EFM32_SDK_BOOT)build/first_stage/gcc/first_stage_btl_$(GBL_VARIANT).s37

ADDITIONAL_BLOBS += $(GBL_FIRST_STAGE)
LIBS += bootloader_$(GBL_VARIANT)_gcc parser_$(GBL_VARIANT)_gcc storage-single_$(GBL_VARIANT)_gcc
DEFINES += EFM32_WATCHDOG_TIMEOUT=0

$(GBL_FIRST_STAGE_SRC): $(EFM32_SDK_BOOT)

$(GBL_FIRST_STAGE): $(GBL_FIRST_STAGE_SRC)
	$(OBJCOPY) -I srec -O elf32-littlearm -B arm --rename-section .sec1=.bin.firststage $< $@

endif
