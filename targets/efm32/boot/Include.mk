#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/boot/Include.mk
#

ifndef BOOTLOADER_BUILD

COMPONENTS += build-gbl

else

TARGETS += gecko-bootloader

LIBS += bootloader_$(GBL_VARIANT)_gcc parser_$(GBL_VARIANT)_gcc storage-single_$(GBL_VARIANT)_gcc

INCLUDE_DIRS += $(EFM32_SDK_MBEDTLS)

endif
