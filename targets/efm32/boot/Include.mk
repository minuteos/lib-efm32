#
# Copyright (c) 2019 triaxis s.r.o.
# Licensed under the MIT license. See LICENSE.txt file in the repository root
# for full license information.
#
# efm32/boot/Include.mk
#

ifneq (bootloader,$(NAME))

COMPONENTS += build-gbl

endif
