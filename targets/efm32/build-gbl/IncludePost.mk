#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/build-gbl/IncludePost.mk
#

GBL_SIGN_KEY ?= $(wildcard $(PROJECT_ROOT)app-sign-key.pem)
GBL_CRYPT_KEY ?= $(wildcard $(PROJECT_ROOT)app-encrypt-key.txt)

.INTERMEDIATE: $(OUTPUT)-app.srec

$(OUTPUT)-app.srec: $(PRIMARY_OUTPUT)
	$(OBJCOPY) -O srec -j ".text*" $< $@

GBL_INPUT = $(OUTPUT)-app.srec

ifneq (,$(GBL_SIGN_KEY))

GBL_INPUT = $(OUTPUT)-signed.srec
GBL_OPTS += --sign $(GBL_SIGN_KEY)

.INTERMEDIATE: $(OUTPUT)-signed.srec

$(OUTPUT)-signed.srec: $(OUTPUT)-app.srec
	$(SI_COMMANDER) convert $< --secureboot --keyfile $(GBL_SIGN_KEY) -o $@

endif

ifneq (,$(GBL_CRYPT_KEY))

GBL_OPTS += --encrypt $(GBL_CRYPT_KEY)

endif

.PHONY: gbl

all: gbl

gbl: $(OUTPUT).gbl

$(OUTPUT).gbl: $(GBL_INPUT)
	$(SI_COMMANDER) gbl create $@ --app $< $(GBL_OPTS)
