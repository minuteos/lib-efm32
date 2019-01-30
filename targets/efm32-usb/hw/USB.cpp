/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/USB.cpp
 */

#include <hw/USB.h>
#include <hw/EMU.h>

#ifdef Ckernel

//! Based on section 37.3.2 and 37.4.1 of EFM32GG11 Reference Manual
async(_USB::CoreEnable)
async_def()
{
    CMU->EnableUSB();
    await(EMU->SetR5VOutputLevel, 33);
    EnablePHYPins();
    USBDEBUG("Core clock and pins enabled");
}
async_end

async(_USB::CoreReset)
async_def()
{
    PCGCCTL &= ~USB_PCGCCTL_STOPPCLK;
    PCGCCTL &= ~(USB_PCGCCTL_PWRCLMP | USB_PCGCCTL_RSTPDWNMODULE);

    // this is undocumented, but EFM32GG11 sets BVALIDOVEN
    // (B-Perheral session valid override enable) on reset, which causes
    // it to fail to be enumerated as a device
    GOTGCTL &= ~(USB_GOTGCTL_BVALIDOVEN | USB_GOTGCTL_BVALIDOVVAL);
    DATTRIM1 |= USB_DATTRIM1_ENDLYPULLUP;

    GRSTCTL |= USB_GRSTCTL_CSFTRST;
    await_mask(GRSTCTL, USB_GRSTCTL_AHBIDLE, USB_GRSTCTL_AHBIDLE);

    // enable DMA and global interrupt mask
    MODMASK(GAHBCFG, _USB_GAHBCFG_DMAEN_MASK | _USB_GAHBCFG_HBSTLEN_MASK | _USB_GAHBCFG_GLBLINTRMSK_MASK,
        USB_GAHBCFG_DMAEN | USB_GAHBCFG_HBSTLEN_INCR8 | USB_GAHBCFG_GLBLINTRMSK);
    
    // reset all interrupts
    IEN = 0;
    IFC = ~0u;
    GINTMSK = 0;
    GINTSTS = ~0u;

    USBDEBUG("Core reset complete");
}
async_end

#endif
