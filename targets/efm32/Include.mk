#
# Copyright (c) 2018 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/Include.mk - Makefile additions for SiLabs EFM32 MCUs
#

EFM32_DEV_ROOT = /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/
EFM32_SDKS = $(EFM32_DEV_ROOT)sdks/gecko_sdk_suite/

# find the latest available version of the SDK
EFM32_SDK_VERSION ?= $(lastword $(sort $(filter-out Simplicity,$(notdir $(wildcard $(EFM32_SDKS)/*)))))

# we need to link the SDK into our build directory, as it typically contains spaces which Makefile cannot handle at all
EFM32_SDK_ORIGIN = $(EFM32_SDKS)$(EFM32_SDK_VERSION)/
EFM32_DEVICE ?= $(filter $(TARGETS),EFM32%g%)
EFM32_SDK_ORIGIN_DEVICE = $(EFM32_SDK_ORIGIN)platform/Device/SiliconLabs/$(EFM32_DEVICE)/
EFM32_SDK_ORIGIN_CMSIS = $(EFM32_SDK_ORIGIN)platform/CMSIS/

EFM32_SDK_DEVICE = $(OUTDIR)efm32-device/
EFM32_SDK_CMSIS = $(OUTDIR)efm32-cmsis/

EFM32_DEVICE_INCLUDE = $(EFM32_SDK_DEVICE)Include/
EFM32_CMSIS_INCLUDE = $(EFM32_SDK_CMSIS)Include/
EFM32_PART_HEADER := $(firstword $(sort $(filter-out Simplicity,$(notdir $(wildcard $(EFM32_SDK_ORIGIN)platform/Device/SiliconLabs/$(EFM32_DEVICE)/Include/$(EFM32_PART))))))

INCLUDE_DIRS += $(EFM32_DEVICE_INCLUDE) $(EFM32_CMSIS_INCLUDE)
DEFINES += EFM32_DEVICE=$(EFM32_DEVICE) EFM32_PART_HEADER=$(EFM32_PART_HEADER)
COMPONENTS += hw

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

prebuild: efm32_sdk

efm32_sdk:
	$(info Using EFM32 SDK in $(EFM32_SDK_ORIGIN), device $(EFM32_DEVICE), part header $(EFM32_PART_HEADER))
	@$(MKDIR) -p $(OUTDIR)
	$(LN) -shf $(EFM32_SDK_ORIGIN_DEVICE) $(EFM32_SDK_DEVICE:/=)
	$(LN) -shf $(EFM32_SDK_ORIGIN_CMSIS) $(EFM32_SDK_CMSIS:/=)
