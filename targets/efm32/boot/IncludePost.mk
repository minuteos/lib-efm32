#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/boot/IncludePost.mk
#

# Disable signing and encryption enforcement if building without signing/encryption

ifdef BOOTLOADER_BUILD

ifeq (,$(GECKO_SIGN_KEY))

DEFINES += GECKO_BOOTLOADER_ENFORCE_SECURE_BOOT=0 GECKO_BOOTLOADER_ENFORCE_SIGNED_UPGRADE=0

endif

ifeq (,$(GECKO_CRYPT_KEY))

DEFINES += GECKO_BOOTLOADER_ENFORCE_ENCRYPTED_UPGRADE=0

endif

endif
