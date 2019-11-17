#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/build-gbl/IncludePost.mk
#

GBL_SIGN_KEY ?= $(wildcard $(PROJECT_ROOT)app-sign-key.pem)
GBL_CRYPT_KEY ?= $(wildcard $(PROJECT_ROOT)app-encrypt-key.txt)

GBL_APP_SREC = $(OBJDIR)gbl-app.srec

$(GBL_APP_SREC): $(PRIMARY_OUTPUT)
	$(OBJCOPY) -O srec -j ".text*" $< $@

GBL_INPUT = $(GBL_APP_SREC)

ifneq (,$(GBL_SIGN_KEY))

GBL_SIGN_SREC = $(OBJDIR)gbl-sign.srec
GBL_INPUT = $(GBL_SIGN_SREC)
GBL_OPTS += --sign $(GBL_SIGN_KEY)
GBL_DEPS += $(GBL_SIGN_KEY)

$(GBL_SIGN_SREC): $(GBL_APP_SREC) $(GBL_SIGN_KEY)
	$(SI_COMMANDER) convert $< --secureboot --keyfile $(GBL_SIGN_KEY) -o $@

endif

ifneq (,$(GBL_CRYPT_KEY))

GBL_OPTS += --encrypt $(GBL_CRYPT_KEY)
GBL_DEPS += $(GBL_CRYPT_KEY)

endif

.PHONY: gbl

all: gbl

gbl: $(OUTPUT).gbl

$(OUTPUT).gbl: $(GBL_INPUT) $(GBL_DEPS)
	$(SI_COMMANDER) gbl create $@ --app $< $(GBL_OPTS)
