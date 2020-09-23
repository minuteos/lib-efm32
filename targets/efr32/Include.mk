#
# Copyright (c) %Y triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efr32/Include.mk
#

EFM32_SDK_ORIGIN_RAIL = $(EFM32_SDK_ORIGIN)platform/radio/rail_lib/

EFM32_SDK_RAIL = $(OBJDIR)efr32-rail/

EFM32_RAIL_INCLUDE = $(EFM32_SDK_RAIL)

INCLUDE_DIRS += $(EFM32_RAIL_INCLUDE) $(EFM32_RAIL_INCLUDE)common $(EFM32_RAIL_INCLUDE)chip/efr32/$(EFM32_RAIL_CHIP) $(EFM32_RAIL_INCLUDE)
LIB_DIRS += $(EFM32_SDK_RAIL)autogen/librail_release/

.PHONY: efr32_rail_sdk

prebuild: efr32_rail_sdk

efr32_rail_sdk: $(EFM32_SDK_RAIL)

$(EFM32_SDK_RAIL): $(OBJDIR)
	@$(LN) -snf $(EFM32_SDK_ORIGIN_RAIL) $(EFM32_SDK_RAIL:/=)
