#
# Copyright (c) 2018 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/Include.mk - Makefile additions for SiLabs EFM32 MCUs
#

EFM32_MAKEFILE := $(call curmake)
EFM32_LIB_DIR = $(call parentdir,$(call parentdir,$(call parentdir,$(EFM32_MAKEFILE))))

EFM32_TOOL_DIR = $(EFM32_LIB_DIR)tools/

SI_INSTALL := $(filter si-install,$(MAKECMDGOALS))

ifeq (,$(wildcard $(EFM32_DEV_ROOT)))
  # try to find simplicity studio in default platform location
  ifeq (Darwin,$(HOST_OS))
    EFM32_DEV_ROOT := $(call escspace,$(wildcard /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/))
  endif
  ifeq (Linux,$(HOST_OS))
    EFM32_DEV_ROOT := $(firstword $(wildcard $(addsuffix /SimplicityStudio_v4/developer/,$(HOME)/.minuteos $(HOME) /opt /usr/local)))
  endif

  ifeq (,$(EFM32_DEV_ROOT))
    ifeq (,$(SI_INSTALL))
      $(error Simplicity Studio not found, use 'make si-install' to download a private copy)
    else
      EFM32_DEV_ROOT := $(HOME)/.minuteos/SimplicityStudio_v4/developer/
    endif
  endif
endif

$(info Simplicity Studio developer root: $(EFM32_DEV_ROOT))

ifneq (,$(wildcard $(EFM32_DEV_ROOT)toolchains/gnu_arm/7.2_2017q4/bin/))
  $(info Using GCC 7.2 from Simplicity Studio)
  SHELL := env 'PATH=$(call unescspace,$(EFM32_DEV_ROOT))toolchains/gnu_arm/7.2_2017q4/bin/:$(PATH)' $(SHELL)
  TOOLCHAIN_PATH := $(call unescspace,$(EFM32_DEV_ROOT))toolchains/gnu_arm/7.2_2017q4/bin/
else
  $(warning Simplicity Studio GCC 7.2 not found, recommend installing it using 'make si-install PACKAGE=com.silabs.ss.toolchain.gnu.arm.7.2.2017.q4')
endif

EFM32_SDKS ?= $(EFM32_DEV_ROOT)sdks/gecko_sdk_suite/

ifeq (,$(wildcard $(SI_COMMANDER)))
  # try to find simplicity commander
  ifeq (Darwin,$(HOST_OS))
    SI_COMMANDER := $(call escspace,$(wildcard $(EFM32_DEV_ROOT)adapter_packs/commander/Commander.app/Contents/MacOS/commander))
  else
    SI_COMMANDER := $(wildcard $(EFM32_DEV_ROOT)adapter_packs/commander/commander)
  endif

  ifeq (,$(SI_COMMANDER))
    ifeq (,$(SI_INSTALL))
      $(error Simplicity Commander not found, use 'make si-install PACKAGE=com.silabs.apack.commander' to install it or set SI_COMMANDER explicitly)
    endif
  endif
endif

# find the latest available version of the SDK
ifeq (,$(EFM32_SDK_VERSION))
  EFM32_SDK_VERSION ?= $(lastword $(sort $(filter-out Simplicity,$(notdir $(wildcard $(EFM32_SDKS)/*)))))
  ifeq (,$(EFM32_SDK_VERSION))
    EFM32_SDK_VERSION = v2.7
    $(warning EFM32_SDK_VERSION not specified explicitly, using recommended version: $(EFM32_SDK_VERSION))
  else
    $(warning EFM32_SDK_VERSION not specified explicitly, using latest available version: $(EFM32_SDK_VERSION))
  endif
endif

# we need to link the SDK into our build directory, as it typically contains spaces which Makefile cannot handle at all
EFM32_SDK_ORIGIN ?= $(EFM32_SDKS)$(EFM32_SDK_VERSION)/
EFM32_DEVICE ?= $(filter $(TARGETS),EFM32%g%)
EFM32_SDK_ORIGIN_DEVICE = $(EFM32_SDK_ORIGIN)platform/Device/SiliconLabs/$(EFM32_DEVICE)/
EFM32_SDK_ORIGIN_CMSIS = $(EFM32_SDK_ORIGIN)platform/CMSIS/
EFM32_SDK_ORIGIN_EMLIB = $(EFM32_SDK_ORIGIN)platform/emlib/
EFM32_SDK_ORIGIN_BOOT = $(EFM32_SDK_ORIGIN)platform/bootloader/
EFM32_SDK_ORIGIN_MBEDTLS = $(EFM32_SDK_ORIGIN)util/third_party/mbedtls/

EFM32_SDK_DEVICE = $(OBJDIR)efm32-device/
EFM32_SDK_CMSIS = $(OBJDIR)efm32-cmsis/
EFM32_SDK_EMLIB = $(OBJDIR)efm32-emlib/
EFM32_SDK_BOOT = $(OBJDIR)efm32-boot/
EFM32_SDK_MBEDTLS = $(OBJDIR)efm32-mbedtls/

EFM32_DEVICE_INCLUDE = $(EFM32_SDK_DEVICE)Include/
EFM32_CMSIS_INCLUDE = $(EFM32_SDK_CMSIS)Include/
EFM32_EMLIB_INCLUDE = $(EFM32_SDK_EMLIB)inc/
EFM32_BOOT_INCLUDE = $(EFM32_SDK_BOOT) $(EFM32_SDK_BOOT)api/
EFM32_MBEDTLS_INCLUDE = $(EFM32_SDK_MBEDTLS)include/ $(EFM32_SDK_MBEDTLS)sl_crypto/include/
EFM32_PART_HEADER := $(firstword $(sort $(filter-out Simplicity,$(notdir $(wildcard $(EFM32_SDK_ORIGIN)platform/Device/SiliconLabs/$(EFM32_DEVICE)/Include/$(EFM32_PART))))))
EFM32_SYSTEM_SOURCE := $(firstword $(sort $(filter-out Simplicity,$(notdir $(wildcard $(EFM32_SDK_ORIGIN)platform/Device/SiliconLabs/$(EFM32_DEVICE)/Source/*.c)))))
EFM32_SYSTEM_SOURCE_PATH := $(EFM32_SDK_DEVICE)Source/$(EFM32_SYSTEM_SOURCE)

# check for default signing and encryption keys
GECKO_SIGN_KEY ?= $(wildcard $(PROJECT_ROOT)app-sign-key.pem)
GECKO_SIGN_KEY_PUB ?= $(wildcard $(PROJECT_ROOT)app-sign-key.pem.pub)
GECKO_SIGN_KEY_TOKENS_PUB ?= $(wildcard $(PROJECT_ROOT)app-sign-key.pem-tokens.txt)
GECKO_CRYPT_KEY ?= $(wildcard $(PROJECT_ROOT)app-encrypt-key.txt)

INCLUDE_DIRS += $(EFM32_DEVICE_INCLUDE) $(EFM32_CMSIS_INCLUDE) $(EFM32_EMLIB_INCLUDE) $(EFM32_BOOT_INCLUDE) $(EFM32_MBEDTLS_INCLUDE)
LIB_DIRS += $(EFM32_SDK_BOOT)build/lib/
ADDITIONAL_SOURCES += $(EFM32_SYSTEM_SOURCE_PATH)
DEFINES += EFM32_DEVICE=$(EFM32_DEVICE) EFM32_PART_HEADER=$(EFM32_PART_HEADER) MBEDTLS_CONFIG_FILE=\"../configs/config-sl-crypto-all-acceleration.h\"
COMPONENTS += hw emlib

ifeq (,$(SI_INSTALL))

# validate paths
ifeq (,$(wildcard $(EFM32_SDK_ORIGIN)))
  $(error Failed to locate EFM32 SDK $(EFM32_SDK_VERSION) in $(EFM32_SDKS), use 'make si-install PACKAGE=com.silabs.sdk.gecko_platform.$(EFM32_SDK_VERSION)' or specify EFM32_SDK_ORIGIN manually)
endif

ifeq (,$(EFM32_DEVICE))
  $(error Failed to autodetect EFM32 device from targets $(TARGETS), please specify manually)
endif

ifeq (,$(EFM32_PART))
  $(error Failed to autodetect EFM32 part, please specify manually)
endif

else

# we're going to install Simplicity Studio and/or packages
ifeq (Darwin,$(HOST_OS))
  SI_STUDIO := $(EFM32_DEV_ROOT)../../MacOS/studio
else
  SI_STUDIO := $(EFM32_DEV_ROOT)../studio
endif

SI_REPOSITORY := https://developer.silabs.com/studio/v4/updates,https://developer.silabs.com/studio/v4/control/stacks/PublicGA/updates
SI_DIRECTOR := $(SI_STUDIO) -nosplash -consolelog -application org.eclipse.equinox.p2.director -r $(SI_REPOSITORY)

endif

.PHONY: efm32_sdk si-install

all: srec

prebuild: efm32_sdk

$(EFM32_SYSTEM_SOURCE_PATH): efm32_sdk

efm32_sdk: $(EFM32_SDK_DEVICE) $(EFM32_SDK_CMSIS) $(EFM32_SDK_EMLIB) $(EFM32_SDK_BOOT) $(EFM32_SDK_MBEDTLS)
	$(info Using EFM32 SDK in $(EFM32_SDK_ORIGIN), device $(EFM32_DEVICE), part header $(EFM32_PART_HEADER))

$(EFM32_SDK_DEVICE)Source/: $(EFM32_SDK_DEVICE:/=)

$(EFM32_SDK_DEVICE): $(OBJDIR)
	@$(LN) -snf $(EFM32_SDK_ORIGIN_DEVICE) $(EFM32_SDK_DEVICE:/=)

$(EFM32_SDK_CMSIS): $(OBJDIR)
	@$(LN) -snf $(EFM32_SDK_ORIGIN_CMSIS) $(EFM32_SDK_CMSIS:/=)

$(EFM32_SDK_EMLIB): $(OBJDIR)
	@$(LN) -snf $(EFM32_SDK_ORIGIN_EMLIB) $(EFM32_SDK_EMLIB:/=)

$(EFM32_SDK_BOOT): $(OBJDIR)
	@$(LN) -snf $(EFM32_SDK_ORIGIN_BOOT) $(EFM32_SDK_BOOT:/=)

$(EFM32_SDK_MBEDTLS) : $(OBJDIR)
	@$(LN) -snf $(EFM32_SDK_ORIGIN_MBEDTLS) $(EFM32_SDK_MBEDTLS:/=)

$(EFM32_DEV_ROOT):
	@$(MKDIR) -p $(EFM32_DEV_ROOT)

si-install: $(EFM32_DEV_ROOT) $(SI_STUDIO)

ifneq (,$(PACKAGE))

si-install:
	$(SI_DIRECTOR) -i $(addsuffix .feature.feature.group,$(PACKAGE))

endif

ifneq (,$(PACKAGE_SEARCH))

si-install:
	$(SI_DIRECTOR) -l $(PACKAGE_SEARCH)

endif

# download simplicity studio itself

ifeq (Linux,$(HOST_OS))

SI_STUDIO_ROOT := $(call parentdir,$(EFM32_DEV_ROOT))
SI_STUDIO_DL_TMP := $(call parentdir,$(SI_STUDIO_ROOT)).download/

$(SI_STUDIO):
	$(info Downoading Simplicity Studio v4)
	rm -rf $(SI_STUDIO_DL_TMP)
	mkdir -p $(SI_STUDIO_DL_TMP)
	wget https://www.silabs.com/documents/login/software/SimplicityStudio-v4.tgz -O - | tar -xzC $(SI_STUDIO_DL_TMP)
	rm -rf $(SI_STUDIO_ROOT)
	mv $(SI_STUDIO_DL_TMP)SimplicityStudio_v4 $(SI_STUDIO_ROOT:/=)
	rm -rf $(SI_STUDIO_DL_TMP)

endif

# helper to upload keys

ifneq (,$(GECKO_SIGN_KEY_TOKENS_PUB)$(GECKO_CRYPT_KEY))

SI_COMMANDER_FLASH_KEYS :=

ifneq (,$(GECKO_SIGN_KEY_TOKENS_PUB))
SI_COMMANDER_FLASH_KEYS += --tokenfile $(GECKO_SIGN_KEY_TOKENS_PUB)
endif

ifneq (,$(GECKO_CRYPT_KEY))
SI_COMMANDER_FLASH_KEYS += --tokenfile $(GECKO_CRYPT_KEY)
endif

.PHONY: flash-keys

flash-keys:
	$(SI_COMMANDER) flash --tokengroup znet $(SI_COMMANDER_FLASH_KEYS) -d $(SI_COMMANDER_DEVICE)

endif

GECKO_SIGN_KEY_PUB ?= $(wildcard $(PROJECT_ROOT)app-sign-key.pem.pub)
GECKO_CRYPT_KEY ?= $(wildcard $(PROJECT_ROOT)app-encrypt-key.txt)
