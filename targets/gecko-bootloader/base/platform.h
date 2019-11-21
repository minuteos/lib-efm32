/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * gecko-bootloader/base/platform.h
 *
 * Injects Gecko Bootloader initialization into startup code
 */

// SystemInit2 starts the application, if present
EXTERN_C void SystemInit2();

#include_next <base/platform.h>

// undefine some standard things that are not actually needed (esp. LFXO)
#undef MONO_US

#undef CORTEX_STARTUP_BEFORE_C_INIT
#define CORTEX_STARTUP_BEFORE_C_INIT  SystemInit2
