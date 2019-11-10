/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/ADC.cpp
 */

#include <hw/ADC.h>

void ADC::ScanSetup(ModeFlags flags, const APORT* ports, size_t count)
{
    uint32_t mask = 0, sel = 0;
    size_t offset = 0;

    for (size_t i = 0; i < count; i++)
    {
        auto p = ports[i];
        if (!i || uint8_t(sel >> offset) != p.ScanInputSel() || uint8_t(mask >> offset) & ~(p.ScanMask() - 1))
        {
            // initialize group
            if (i)
            {
                // move to next group if a different selector is needed, or if there are inputs above the requested one already selected
                offset += 8;
                if (offset == 32)
                {
                    // cannot scan all the ports
                    ASSERT(false);
                    break;
                }
            }
            sel |= p.ScanInputSel() << offset;
        }
        // add to group mask
        mask |= p.ScanMask() << offset;
    }

    SCANCTRL = flags;
    SCANINPUTSEL = sel;
    SCANMASK = mask;
}
