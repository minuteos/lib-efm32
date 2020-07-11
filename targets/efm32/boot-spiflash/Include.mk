#
# Copyright (c) 2020 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/boot-spiflash/Include.mk
#

COMPONENTS += boot
BOOT_COMPONENTS += boot-spiflash kernel storage bus

ifeq (bootloader,$(NAME))

DEFINES += GECKO_BOOTLOADER_STORAGE_EXTERNAL EFM32_USE_GPIO_SPI KERNEL_SYNC_ONLY MEMPOOL_NO_MALLOC

endif
