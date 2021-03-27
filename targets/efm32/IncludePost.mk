#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/IncludePost.mk
#
# additional targets for EFM32, such as signed, encrypted, etc...
#

GECKO_APP_SREC = $(OUTPUT)-app.s37
FLASH_OUTPUT = $(GECKO_APP_SREC)

.PHONY: app-srec

all: app-srec

app-srec: $(GECKO_APP_SREC)

$(GECKO_APP_SREC): $(PRIMARY_OUTPUT)
	$(OBJCOPY) -O srec -j ".text*" -j ".data*" --srec-forceS3 $< $@

ifneq (,$(GECKO_SIGN_KEY))

ifneq (,$(GECKO_SIGN_KEY_PUB))

GECKO_SIGN_VERIFY_OPT = --verify $(GECKO_SIGN_KEY_PUB)

endif

DEFINES += GECKO_SIGNATURE

GECKO_APP_SIGNED_SREC = $(OUTPUT)-app-signed.s37
GECKO_SIGNED_PRIMARY = $(OUTPUT)-signed$(PRIMARY_EXT)
GECKO_SIGNED_SREC = $(OUTPUT)-signed.s37
LAUNCH_OUTPUT = $(GECKO_SIGNED_PRIMARY)
FLASH_OUTPUT = $(GECKO_SIGNED_SREC)

.PHONY: signed-app-srec signed-main signed-srec

all: signed-app-srec signed-main signed-srec

signed-app-srec: $(GECKO_APP_SIGNED_SREC)

signed-main: $(GECKO_SIGNED_PRIMARY)

signed-srec: $(GECKO_SIGNED_SREC)

$(GECKO_APP_SIGNED_SREC): $(GECKO_APP_SREC) $(GECKO_SIGN_KEY)
	$(SI_COMMANDER) convert $< --secureboot --keyfile $(GECKO_SIGN_KEY) $(GECKO_SIGN_VERIFY_OPT) -o $@

$(GECKO_SIGNED_PRIMARY): $(PRIMARY_OUTPUT) $(GECKO_APP_SIGNED_SREC)
	$(EFM32_TOOL_DIR)patch_elf $< $(GECKO_APP_SIGNED_SREC) $@ $(OBJCOPY) $(OBJDUMP)

$(GECKO_SIGNED_SREC): $(GECKO_SIGNED_PRIMARY)
	$(OBJCOPY) -O srec --srec-forceS3 $< $@

endif

.PHONY: flash-program

flash-program: $(FLASH_OUTPUT)
	$(SI_COMMANDER) flash --speed 4000 $(FLASH_OUTPUT) -d $(SI_COMMANDER_DEVICE)
