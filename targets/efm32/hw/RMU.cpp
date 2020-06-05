/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/RMU.cpp
 */

#include <hw/RMU.h>
#include <hw/EMU.h>

#ifdef Ckernel
#include <kernel/kernel.h>
#endif

_RMU::Cause _RMU::s_cause;

void _RMU::Configure()
{
    s_cause = (Cause)RSTCAUSE;
#if !BOOTLOADER
    CMD = RMU_CMD_RCCLR;
#endif

#if TRACE
    DBGC("RMU", "Reset cause:");
    auto cause = s_cause;
#define TRACE_CAUSE(name, flag) if (!!(cause & Cause::flag)) { cause &= ~Cause::flag; _DBGCHAR(' '); _DBG(name); }
    TRACE_CAUSE("POR", PowerOn);
    TRACE_CAUSE("BOD", Brownout);
    TRACE_CAUSE("EXT", External);
    TRACE_CAUSE("LCK", Lockup);
    TRACE_CAUSE("SYS", System);
    TRACE_CAUSE("WDT", Watchdog);
    TRACE_CAUSE("BAK", Backup);
    TRACE_CAUSE("EM4", EM4Wakeup);
#undef TRACE_CAUSE
    if (!!cause)
    {
        _DBG(" +%X", cause);
    }
    _DBGCHAR('\n');
#endif

#if defined(Ckernel) && !defined(HAS_BOOTLOADER)
    if (ResetCauseWatchdog())
        kernel::resetCause = kernel::ResetCause::Watchdog;
    else if (ResetCauseBackup())
        kernel::resetCause = kernel::ResetCause::Backup;
    else if (ResetCauseEM4())
        kernel::resetCause = kernel::ResetCause::Hibernation;
    else if (ResetCauseSystem())
        kernel::resetCause = kernel::ResetCause::Software;
    else if (ResetCauseExternal())
        kernel::resetCause = kernel::ResetCause::Hardware;
    else if (ResetCauseBrownout())
        kernel::resetCause = kernel::ResetCause::Brownout;
#endif

#if !defined(EFM32_NO_ABSOLUTE_TIME) && defined(_RMU_CTRL_WDOGRMODE_MASK)
    // do not reset the RTCC, the only way to reset it is POR
    MODMASK(CTRL, _RMU_CTRL_WDOGRMODE_MASK | _RMU_CTRL_SYSRMODE_MASK | _RMU_CTRL_LOCKUPRMODE_MASK | _RMU_CTRL_PINRMODE_MASK,
        RMU_CTRL_WDOGRMODE_LIMITED | RMU_CTRL_SYSRMODE_LIMITED | RMU_CTRL_LOCKUPRMODE_LIMITED | RMU_CTRL_PINRMODE_LIMITED);
#endif

#ifdef EMU_CMD_EM4UNLATCH
    if (!ResetCauseEM4())
    {
        // unlatch EMU to be safe - it is possible that another reset occured after clearing the EM4WU bit in RSTCAUSE, but before the latch is lifted
        EMU->Unlatch();
    }
#endif
}
