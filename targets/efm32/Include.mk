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

EFM32_DEV_ROOT ?= /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/
EFM32_SDKS ?= $(EFM32_DEV_ROOT)sdks/gecko_sdk_suite/
SI_COMMANDER ?= $(EFM32_DEV_ROOT)adapter_packs/commander/Commander.app/Contents/MacOS/commander

ifeq (,$(SI_COMMANDER))
  # hopefully will be in the path
  SI_COMMANDER = commander
endif

# find the latest available version of the SDK
EFM32_SDK_VERSION ?= $(lastword $(sort $(filter-out Simplicity,$(notdir $(wildcard $(EFM32_SDKS)/*)))))

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
EFM32_MBEDTLS_INCLUDE = $(EFM32_SDK_MBEDTLS)include/
EFM32_PART_HEADER := $(firstword $(sort $(filter-out Simplicity,$(notdir $(wildcard $(EFM32_SDK_ORIGIN)platform/Device/SiliconLabs/$(EFM32_DEVICE)/Include/$(EFM32_PART))))))
EFM32_SYSTEM_SOURCE := $(firstword $(sort $(filter-out Simplicity,$(notdir $(wildcard $(EFM32_SDK_ORIGIN)platform/Device/SiliconLabs/$(EFM32_DEVICE)/Source/*.c)))))
EFM32_SYSTEM_SOURCE_PATH := $(EFM32_SDK_DEVICE)Source/$(EFM32_SYSTEM_SOURCE)

INCLUDE_DIRS += $(EFM32_DEVICE_INCLUDE) $(EFM32_CMSIS_INCLUDE) $(EFM32_EMLIB_INCLUDE) $(EFM32_BOOT_INCLUDE) $(EFM32_MBEDTLS_INCLUDE)
LIB_DIRS += $(EFM32_SDK_BOOT)build/lib/
ADDITIONAL_SOURCES += $(EFM32_SYSTEM_SOURCE_PATH)
DEFINES += EFM32_DEVICE=$(EFM32_DEVICE) EFM32_PART_HEADER=$(EFM32_PART_HEADER)
COMPONENTS += hw emlib

ifeq (,$(wildcard $(EFM32_SDK_ORIGIN)))
  $(error Failed to locate EFM32 SDK in $(EFM32_SDKS), please specify EFM32_SDK_ORIGIN manually)
endif

ifeq (,$(EFM32_DEVICE))
  $(error Failed to autodetect EFM32 device from targets $(TARGETS), please specify manually)
endif

ifeq (,$(EFM32_PART))
  $(error Failed to autodetect EFM32 part, please specify manually)
endif

.PHONY: efm32_sdk

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
