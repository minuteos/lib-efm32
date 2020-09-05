/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * emlib.c
 *
 * Makes the entire SiLabs emlib available
 *
 * Unused symbols will be discarded by the linker
 */

#define EM_MSC_RUN_FROM_FLASH

#include <base/base.h>

// the sources are next to the "inc" directory where the em_ headers are located
#include <../src/em_cryotimer.c>
#include <../src/em_dac.c>
#include <../src/em_dma.c>
#include <../src/em_acmp.c>
#include <../src/em_adc.c>
#include <../src/em_gpio.c>
#include <../src/em_usart.c>
#include <../src/em_crypto.c>
#if _SILICON_LABS_32B_SERIES < 2
#include <../src/em_letimer.c>
#endif
#include <../src/em_lcd.c>
#include <../src/em_qspi.c>
#include <../src/em_opamp.c>
#include <../src/em_ldma.c>
#include <../src/em_can.c>
#include <../src/em_pcnt.c>
#include <../src/em_aes.c>
#include <../src/em_msc.c>
#include <../src/em_gpcrc.c>
#include <../src/em_vdac.c>
#include <../src/em_cmu.c>
#include <../src/em_system.c>
#include <../src/em_timer.c>
#include <../src/em_core.c>
#include <../src/em_rtc.c>
#include <../src/em_assert.c>
#include <../src/em_rmu.c>
#include <../src/em_emu.c>
#include <../src/em_burtc.c>
#include <../src/em_prs.c>
#include <../src/em_vcmp.c>
#include <../src/em_csen.c>
#include <../src/em_idac.c>
#include <../src/em_wdog.c>
#include <../src/em_ebi.c>
#include <../src/em_leuart.c>
#include <../src/em_lesense.c>
#include <../src/em_rtcc.c>
#include <../src/em_dbg.c>
