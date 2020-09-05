/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/GPIO_DriveConfig.h
 * 
 * Shared configuration for GPIO slewrate control on Series 1 and 2
 */

#pragma once

#include <base/base.h>

#ifdef _GPIO_P_CTRL_SLEWRATE_MASK

//! Determines if GPIO drive control is available
#define EFM32_GPIO_DRIVE_CONTROL 1

//! Weak (1 mA) GPIO drive with the specified slew rate (between @ref EFM32_GPIO_SLEW_SLOWEST and @ref EFM32_GPIO_SLEW_FASTEST)
#define EFM32_GPIO_DRIVE_WEAK(slew)    (((slew) << 4) | 1)
//! Strong (10 mA) GPIO drive with the specified slew rate (between @ref EFM32_GPIO_SLEW_SLOWEST and @ref EFM32_GPIO_SLEW_FASTEST)
#define EFM32_GPIO_DRIVE_STRONG(slew)    ((slew) << 4)

#define EFM32_GPIO_DRIVE_SETUP(drive, alt)    (((alt) << 16) | (drive))

//! Represents the slowest available slew rate
#define EFM32_GPIO_SLEW_SLOWEST    0
//! Represents the default slew rate after configuration
#define EFM32_GPIO_SLEW_DEFAULT    4
//! Represents the default slew rate after reset
#define EFM32_GPIO_SLEW_RESET      5
//! Represents the fastest available slew rate
#define EFM32_GPIO_SLEW_FASTEST    7

#include <efm32_gpio_config.h>

//! Default GPIO drive configuration after reset
#define EFM32_GPIO_DRIVE_RESET      EFM32_GPIO_DRIVE_SETUP(EFM32_GPIO_DRIVE_STRONG(EFM32_GPIO_SLEW_RESET), EFM32_GPIO_DRIVE_STRONG(EFM32_GPIO_SLEW_RESET))

#ifndef EFM32_GPIO_ALT_DRIVE
//! Alternate GPIO drive configuration
#define EFM32_GPIO_ALT_DRIVE        EFM32_GPIO_DRIVE_WEAK(EFM32_GPIO_SLEW_DEFAULT)
#endif

#ifndef EFM32_GPIO_DRIVE
//! Primary GPIO drive configuration
#define EFM32_GPIO_DRIVE            EFM32_GPIO_DRIVE_STRONG(EFM32_GPIO_SLEW_DEFAULT)
#endif

#ifndef EFM32_GPIO_DRIVE_DEFAULT
//! Default application specific GPIO drive configuration
#define EFM32_GPIO_DRIVE_DEFAULT    EFM32_GPIO_DRIVE_SETUP(EFM32_GPIO_DRIVE, EFM32_GPIO_ALT_DRIVE)
#endif

#endif
