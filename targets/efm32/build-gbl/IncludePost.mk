#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/build-gbl/IncludePost.mk
#

ifneq (,$(GECKO_SIGN_KEY))

GBL_INPUT = $(GECKO_APP_SIGNED_SREC)
GBL_OPTS += --sign $(GECKO_SIGN_KEY)
GBL_DEPS += $(GECKO_SIGN_KEY)

else

GBL_INPUT = $(GECKO_APP_SREC)

endif

ifneq (,$(GECKO_CRYPT_KEY))

GBL_OPTS += --encrypt $(GECKO_CRYPT_KEY)
GBL_DEPS += $(GECKO_CRYPT_KEY)

endif

GECKO_GBL = $(OUTPUT).gbl

.PHONY: gbl

all: gbl

gbl: $(GECKO_GBL)

$(GECKO_GBL): $(GBL_INPUT) $(GBL_DEPS)
	$(SI_COMMANDER) gbl create $@ --app $< $(GBL_OPTS)
