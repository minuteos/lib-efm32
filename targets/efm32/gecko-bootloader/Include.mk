#
# Copyright (c) %Y triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/gecko-bootloader/Include.mk
#

EFM32_SDK_ORIGIN_BOOT = $(EFM32_SDK_ORIGIN)platform/bootloader/

EFM32_SDK_BOOT = $(OUTDIR)efm32-boot/

EFM32_BOOT_INCLUDE = $(EFM32_SDK_BOOT)api/

INCLUDE_DIRS += $(EFM32_BOOT_INCLUDE)

.PHONY: efm32_boot

prebuild: efm32_boot

efm32_boot: efm32_sdk
	@$(LN) -snf $(EFM32_SDK_ORIGIN_BOOT) $(EFM32_SDK_BOOT:/=)
