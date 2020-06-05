/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * hw/RMU.h
 */

#pragma once

#include <base/base.h>

#undef RMU
#define RMU	CM_PERIPHERAL(_RMU, RMU_BASE)

class _RMU : public RMU_TypeDef
{
public:
    enum struct Cause
    {
        PowerOn = RMU_RSTCAUSE_PORST,
#ifdef RMU_RSTCAUSE_AVDDBOD
        Brownout = RMU_RSTCAUSE_AVDDBOD | RMU_RSTCAUSE_DVDDBOD | RMU_RSTCAUSE_DECBOD,
#elif defined(RMU_RSTCAUSE_BODAVDD0)
        Brownout = (RMU_RSTCAUSE_BODAVDD0 | RMU_RSTCAUSE_BODAVDD1 | RMU_RSTCAUSE_BODREGRST | RMU_RSTCAUSE_BODUNREGRST),
#else
        Brownout = 0,
#endif
        External = RMU_RSTCAUSE_EXTRST,
        Lockup = RMU_RSTCAUSE_LOCKUPRST,
        System = RMU_RSTCAUSE_SYSREQRST,
        Watchdog = RMU_RSTCAUSE_WDOGRST,
#ifdef RMU_RSTCAUSE_BUMODERST
        Backup = RMU_RSTCAUSE_BUMODERST,
#else
        Backup = 0,
#endif
        EM4Wakeup = RMU_RSTCAUSE_EM4RST,
    };

    DECLARE_FLAG_ENUM(Cause);

    void Configure();

    Cause ResetCause() { return s_cause; }

    bool ResetCauseEM4() { return !!(s_cause & Cause::EM4Wakeup); }
    bool ResetCauseBackup() { return !!(s_cause & Cause::Backup); }
    bool ResetCauseWatchdog() { return !!(s_cause & Cause::Watchdog); }
    bool ResetCauseSystem() { return !!(s_cause & Cause::System); }
    bool ResetCauseLockup() { return !!(s_cause & Cause::Lockup); }
    bool ResetCauseExternal() { return !!(s_cause & Cause::External); }
    bool ResetCauseBrownout() { return !!(s_cause & Cause::Brownout); }
    bool ResetCausePowerOn() { return !!(s_cause & Cause::PowerOn); }

private:
    static Cause s_cause;
};

DEFINE_FLAG_ENUM(_RMU::Cause);

