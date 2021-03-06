/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/hw/GPIO.h
 */

//! @file efm32/hw/GPIO.h
//! @brief This file contains convenience wrappers around EFM32 GPIO pins and ports

#pragma once

#include <base/base.h>

#ifdef Ckernel
#include <kernel/kernel.h>
#endif

#include <hw/CMU.h>

#include <em_gpio.h>        // let EMLIB detect the actual port configuration

#if defined(_SILICON_LABS_32B_SERIES_1) && !defined(_EFM32_GIANT_FAMILY)

// GPIO configuration of EFM32J/PG1 and EFR32 Series 1 MCUs
// the table contains offsets that linearize the pins so that PA0-PA5 = 0-5, PB11-PB15 = 6-11, ...

//! Table for converting port and pin index to a linear pin index
#define EFM32_GPIO_LINEAR_INDEX ((const signed char[]){ \
    0,      /* PA0  ( 0) - PA5  ( 5) */ \
    6 - 11, /* PB11 ( 6) - PB15 (10) */ \
    11 - 6, /* PC6  (11) - PC11 (16) */ \
    17 - 9, /* PD9  (17) - PD15 (23) */ \
    0,      /* PEx */ \
    24 - 0, /* PF0  (24) - PF7  (31) */ \
})

#endif

#ifdef _SILICON_LABS_32B_SERIES_2

typedef GPIO_PORT_TypeDef GPIO_P_TypeDef;

#endif

#include <hw/GPIO_DriveConfig.h>

#undef GPIO
#define GPIO    CM_PERIPHERAL(GPIOBlock, GPIO_BASE)
#define GPIO_P(n) CM_PERIPHERAL(GPIOPort, GPIO_BASE + offsetof(GPIO_TypeDef, P) + sizeof(GPIO_P_TypeDef) * (n))

class GPIOBlock;
class GPIOPort;
class GPIOPinID;

class APORT
{
    union
    {
        uint8_t value;
        struct
        {
            uint8_t scanBit : 3;
            uint8_t scanInput : 5;
        };
    };

protected:
    constexpr APORT(uint8_t value) : value(value) {}

    static unsigned BusMask(unsigned value)
    {
        unsigned num = value >> 5;      // number of port, i.e. 0-4
        bool y = (value & 1) == (num & 1);	// odd ports have X channel first, even ports have Y channel first
        return 1 << (num << 1) << y;	// BUS<num>[XY]
    }

public:
    //! APORT fixed internal connections
    enum Fixed
    {
        AVDD = 224,
        DVDD = 226,
        IOVDD = 229,
        DACOUT0 = 242,
        DACOUT1 = 243,
        VLP = 251,
        VBDIV = 252,
        VADIV = 253,
        VDD = 254,
        VSS = 255,
    };

    constexpr operator uint8_t() const { return value; }

    //! Gets the SCANINPUTSEL value for this port
    uint32_t ScanInputSel() const { return scanInput; }
    //! Gets the SCANMASK value for this port
    uint32_t ScanMask() const { return BIT(scanBit); }
    //! Gets the bus mask (e.g. APORTREQ, APORTMASTERDIS) value for this port
    uint32_t BusMask() const { return BusMask(value); }

    friend class GPIOPinID;
};

//! Compact identification of a GPIO pin
class GPIOPinID
{
    union
    {
        uint8_t id;
        struct
        {
            uint8_t port : 4;
            uint8_t index : 4;
        };
    };

public:
    //! Creates an invalid GPIOPinID
    //! Creates a GPIOPinID from the provided port (0-14) and pin index (0-15)
    constexpr GPIOPinID(int port, int index) : port(port + 1), index(index) {}
    //! Creates a GPIOPinID from the provided compact representation
    constexpr GPIOPinID(uint8_t id) : id(id) {}

    //! Checks if this is a valid GPIO pin
    constexpr bool IsValid() const { return !!id; }
    //! Gets the port index (0-14, 0=A)
    constexpr int Port() const { return port - 1; }
    //! Gets the port index (0-15)
    constexpr int Pin() const { return index; }

    //! Gets the numeric representation of the Pin ID
    constexpr operator uint8_t() const { return id; }

private:
#ifdef EFM32_GPIO_LINEAR_INDEX
    //! APORT mapping for EFM32 Series 1 PINs
    ALWAYS_INLINE constexpr int APORTOffset() const
    {
        switch (port)
        {
            case 1: return 96 + 8;  // PORT A = APORT3/4 CH8-13 for PA0-PA5 (+8)
            case 2: return 96 + 16; // PORT B = APORT3/4 CH27-31 for PB11-15 (+16)
            case 3: return 32 + 0;  // PORT C = APORT1/2 CH6-11 for PC6-11 (+0)
            case 4: return 96 - 8;  // PORT D = APORT3/4 CH1-7 for PD9-15 (-8)
            case 6: return 32 + 16; // PORT F = APORT1/2 CH16-23 for PF0-7 (+16)
        }
        return 0;
    }

#define EFM32_GPIO_APORT_AVAILABLE  1

#elif defined(_SILICON_LABS_32B_SERIES_1) && defined(_EFM32_GIANT_FAMILY)
    //! APORT mapping for EFM32GG11 pins
    ALWAYS_INLINE constexpr int APORTOffset() const
    {
        switch (port)
        {
            case 1: return 32 + 0;  // PORT A = APORT1/2 CH0-15 for PA0-PA15 (+0)
            case 2: return 32 + 16; // PORT B = APORT1/2 CH16-CH31 for PB0-PB15 (+16)
            case 5: return 96 + 0;  // PORT E = APORT3/4 CH0-15 for PE0-PE15 (+0)
            case 6: return 96 + 16; // PORT F = APORT3/4 CH16-31 for PF0-PF15 (+16)
        }
        return 0;
    }

#define EFM32_GPIO_APORT_AVAILABLE  1

#endif

public:
#if EFM32_GPIO_APORT_AVAILABLE
    //! Gets the APORTX channel for this pin
    ALWAYS_INLINE constexpr APORT APORTX() const { return index + APORTOffset() + (index & 1) * 32; }
    //! Gets the APORTY channel for this pin
    ALWAYS_INLINE constexpr APORT APORTY() const { return index + APORTOffset() + !(index & 1) * 32; }
#endif
};

//! Location lookup table type
typedef const GPIOPinID* GPIOLocations_t;
//! Location lookup table definition
#define GPIO_LOC(...)    ((const GPIOPinID[]){__VA_ARGS__, 0})

//! Representation of a GPIO pin
class GPIOPin
{
    GPIOPort* port;
    uint32_t mask;

public:
    //! Creates a new GPIOPin instance from the specified @p port and @p mask
    /*!
     * @remark note that @p mask is not the port index, but a bitmask to be
     * used over port registers. Use the @link PA P<port>(<pin>) @endlink macros to obtain GPIOPin
     * instances.
     */
    constexpr GPIOPin(GPIOPort* port, uint32_t mask) : port(port), mask(mask) {}

    //! Gets the GPIOPort to which the GPIOPin belongs
    constexpr GPIOPort& Port() const { return *port; }
    //! Gets the bitmask of the GPIOPin
    constexpr const uint32_t& Mask() const { return mask; }
    //! Gets the index of the GPIOPin
    /*! If there are multiple bits set in the Mask, the index of the lowest one is returned */
    constexpr uint32_t Index() const { return __builtin_ctz(mask); }

    //! Gets a compact location representation GPIOPinID of the GPIOPin
    /*! Use the @link pA p<port>(<pin>) @endlink macros to get GPIOPinID instances when possible */
    constexpr GPIOPinID GetID() const;
    //! Checks if this is a valid GPIO pin
    constexpr bool IsValid() const { return mask != 0; }
    //! Gets the pin name
    /*! @warning This function is meant mostly for diagnostics and is not reentrant */
    const char* Name() const;

    //! Specifies the drive mode of the GPIOPin
    enum Mode
    {
        //! The pin is not driven in any way (still can be used as an analog input)
        Disabled = GPIO_P_MODEL_MODE0_DISABLED,
        //! The pin is configured as a digital input
        Input = GPIO_P_MODEL_MODE0_INPUT,
        //! The pin is configured as a digital input with pull-up or pull-down (determined by output state)
        InputPull = GPIO_P_MODEL_MODE0_INPUTPULL,
        //! The pin is configured as a digital output with
        PushPull = GPIO_P_MODEL_MODE0_PUSHPULL,
        //! The pin is configured with only high-side driver enabled when output is high; an external pull-down is expected
        WiredOr = GPIO_P_MODEL_MODE0_WIREDOR,
        //! The pin is configured with only high-side driver enabled when output is high, internal pull-down is active
        WiredOrPullDown = GPIO_P_MODEL_MODE0_WIREDORPULLDOWN,
        //! The pin is configured with only low-side driver enabled when output is low; an external pull-up is expected
        WiredAnd = GPIO_P_MODEL_MODE0_WIREDAND,
        //! The pin is configured with only low-side driver enabled when output is low, internal pull-up is active
        WiredAndPullUp = GPIO_P_MODEL_MODE0_WIREDANDPULLUP,
        ModeMask = _GPIO_P_MODEL_MODE0_MASK,

        //! The pin is set high before enabling output
        FlagSet = 0x10,
        //! Glitch filter is enabled for the pin
        FlagFilter = 0x20,
        //! The pin uses alternate drive strength configuration
        FlagAltDrive = 0x40,
#ifdef _GPIO_P_OVTDIS_MASK
        //! Overvoltage protection is disabled for the pin
        FlagNoOvervoltage = 0x80,
#else
        //! Overvoltage protection is disabled for the pin (not supported for this MCU)
        FlagNoOvervoltage = 0,
#endif

        //! Helper for an input with internal pull-down enabled
        InputPullDown = InputPull,
        //! Helper for an input with internal pull-up enabled
        InputPullUp = InputPull | FlagSet,
    };

    //! Configures the GPIOPin as a digital input
    void ConfigureDigitalInput() const { Configure(Input); }
    //! Configures the GPIOPin as a digital input with internal pull-down (@p pullUp == @c false) or pull-up (@p pullUp == @c true) enabled
    void ConfigureDigitalInput(bool pullUp) const { Configure(Mode(InputPull | (pullUp * FlagSet))); }
    //! Configures the GPIOPin as a digital output
    void ConfigureDigitalOutput(bool set = false, bool alt = false) const { Configure(Mode(PushPull | (set * FlagSet) | (alt * FlagAltDrive))); }
    //! Configures the GPIOPin as an open drain output
    void ConfigureOpenDrain(bool set = true) const { Configure(Mode(WiredAndPullUp | (set * FlagSet))); }
    //! Configures the GPIOPin as an analog input
    void ConfigureAnalog() const { Configure(FlagNoOvervoltage); }
    //! Disables the GPIOPin
    void Disable() const { Configure(Disabled); }

    //! Configures the GPIOPin with the specified drive @ref Mode
    void Configure(Mode mode) const;
#if defined(_SILICON_LABS_32B_SERIES_1)
    //! Configures the GPIOPin for an alternate function, selecting from the provided lookup table
    /*! @p route parameter determines both ROUTEPEN register bit and field offset in the ROUTELOC registers */
    ALWAYS_INLINE void ConfigureAlternate(Mode mode, volatile uint32_t& routepen, uint8_t route, GPIOLocations_t locations) const { ConfigureAlternate(mode, routepen, route, route, locations); }
    //! Configures the GPIOPin for an alternate function, selecting from the provided lookup table
    /*! Separate ROUTEPEN (@p route) and ROUTELOC (@p locIndex) offset can be specified */
    void ConfigureAlternate(Mode mode, volatile uint32_t& routepen, uint8_t route, uint8_t locIndex, GPIOLocations_t locations) const;
#elif defined(_SILICON_LABS_32B_SERIES_2)

    template<typename T> static volatile uint32_t& __ROUTEEN(T* array, unsigned index) { return array[index].ROUTEEN; }
    template<typename T> static volatile uint32_t& __ROUTEEN(T& single, unsigned index) { return single.ROUTEEN; }

//! Helper for providing the required arguments to ConfigureAlternate
#define GPIO_ROUTE_ARGS(peripheral, pin) GPIOPin::__ROUTEEN(GPIO->peripheral ## ROUTE, Index()), _GPIO_ ## peripheral ## _ROUTEEN_ ## pin ## PEN_SHIFT, offsetof(GPIO_ ## peripheral ## ROUTE_TypeDef, pin ## ROUTE)
//! Helper for providing the required arguments to ConfigureAlternate for pins that don't have a corresponding bit in ROUTEEN
#define GPIO_ROUTE_ARGS_NOPEN(peripheral, pin) GPIOPin::__ROUTEEN(GPIO->peripheral ## ROUTE, Index()), 32, offsetof(GPIO_ ## peripheral ## ROUTE_TypeDef, pin ## ROUTE)

    //! Configures the GPIOPin for an alternate function
    /*! @p route parameter determines both ROUTEPEN register bit and field offset in the ROUTELOC registers */
    template<typename T> ALWAYS_INLINE void ConfigureAlternate(Mode mode, volatile T& cfgGroup, unsigned penBit, unsigned routeOffset) const { ConfigureAlternate(mode, cfgGroup.ROUTEEN, penBit, routeOffset); }
    //! Configures the GPIOPin for an alternate function, selecting from the provided lookup table
    /*! Separate ROUTEPEN (@p route) and ROUTELOC (@p locIndex) offset can be specified */
    void ConfigureAlternate(Mode mode, volatile uint32_t& routeen, unsigned penBit, unsigned routeOffset) const;
#endif
    //! Configures the GPIOPin for wakeup from EM4
    void ConfigureWakeFromEM4(bool level = false) const;

    //! Enables edge interrupt generation
    /*! @returns the mask to the interrupt registers allocated for the pin */
    uint32_t EnableInterrupt(bool rising, bool falling) const;

#ifdef EFM32_GPIO_LINEAR_INDEX
    //! Gets the linear index of the pin (available on some MCUs)
    constexpr unsigned GetLinearIndex(unsigned offset) const;
    //! Configure the GPIOPin for an alternate function with full multiplexing (available on some MCUs)
    /*! @p route parameter determines both ROUTEPEN register bit and field offset in the ROUTELOC registers */
    ALWAYS_INLINE void ConfigureAlternate(Mode mode, volatile uint32_t& routepen, uint8_t route, unsigned locOffset) const { ConfigureAlternate(mode, routepen, route, route, locOffset); }
    //! Configure the GPIOPin for an alternate function with full multiplexing (available on some MCUs)
    /*! Separate ROUTEPEN (@p route) and ROUTELOC (@p locIndex) offset can be specified */
    void ConfigureAlternate(Mode mode, volatile uint32_t& routepen, uint8_t route, uint8_t locIndex, unsigned locOffset) const;
#endif

    //! Gets the location index of the GPIOPin from the provided lookup table
    ALWAYS_INLINE int GetLocation(GPIOLocations_t locations) const { return GetLocation(locations, GetID()); }
    //! Gets the location index of the specified GPIOPinID from the provided lookup table
    static int GetLocation(GPIOLocations_t locations, GPIOPinID id);

    //! Gets the input state of the GPIOPin
    bool Get() const;
    //! Sets the output of the GPIOPin to a logical 1
    void Set() const;
    //! Reset the output of the GPIOPin to a logical 0
    void Res() const;
    //! Sets the output of the GPIOPin to the desired @p state
    void Set(bool state) const;
    //! Toggles the output of the GPIOPin
    void Toggle() const;
#ifdef Ckernel
    //! Waits for the pin to have the specified state
    async(WaitFor, bool state, Timeout timeout = Timeout::Infinite);
#endif

    //! Gets the input state of the GPIOPin
    operator bool() const { return Get(); }

    //! Checks if two GPIOPin instnaces refer to the same pin
    ALWAYS_INLINE bool operator ==(const GPIOPin& other) const { return &port == &other.port && mask == other.mask; }
    //! Checks if two GPIOPin instnaces refer to different pins
    ALWAYS_INLINE bool operator !=(const GPIOPin& other) const { return &port != &other.port || mask != other.mask; }
};

//! Representation of a GPIO port (group of pins)
class GPIOPort : public GPIO_P_TypeDef
{
public:
#if _GPIO_PORT_A_PIN_COUNT
    //! Gets a GPIOPin instance for the specified pin on port A
    static constexpr GPIOPin A(int pin) { return GPIOPin(GPIO_P(0), BIT(pin)); }
    //! Gets a GPIOPin instance for the specified pin on port A
    #define PA(n)    GPIOPort::A(n)
    //! Gets a GPIOPinID for the specified pin on port A
    #define pA(n)    GPIOPinID(0, (n))
#endif

#if _GPIO_PORT_B_PIN_COUNT
    //! Gets a GPIOPin instance for the specified pin on port B
    static constexpr GPIOPin B(int pin) { return GPIOPin(GPIO_P(1), BIT(pin)); }
    //! Gets a GPIOPin instance for the specified pin on port B
    #define PB(n)    GPIOPort::B(n)
    //! Gets a GPIOPinID for the specified pin on port B
    #define pB(n)    GPIOPinID(1, (n))
#endif

#if _GPIO_PORT_C_PIN_COUNT
    //! Gets a GPIOPin instance for the specified pin on port C
    static constexpr GPIOPin C(int pin) { return GPIOPin(GPIO_P(2), BIT(pin)); }
    //! Gets a GPIOPin instance for the specified pin on port C
    #define PC(n)    GPIOPort::C(n)
    //! Gets a GPIOPinID for the specified pin on port C
    #define pC(n)    GPIOPinID(2, (n))
#endif

#if _GPIO_PORT_D_PIN_COUNT
    //! Gets a GPIOPin instance for the specified pin on port D
    static constexpr GPIOPin D(int pin) { return GPIOPin(GPIO_P(3), BIT(pin)); }
    //! Gets a GPIOPin instance for the specified pin on port D
    #define PD(n)    GPIOPort::D(n)
    //! Gets a GPIOPinID for the specified pin on port D
    #define pD(n)    GPIOPinID(3, (n))
#endif

#if _GPIO_PORT_E_PIN_COUNT
    //! Gets a GPIOPin instance for the specified pin on port E
    static constexpr GPIOPin E(int pin) { return GPIOPin(GPIO_P(4), BIT(pin)); }
    //! Gets a GPIOPin instance for the specified pin on port E
    #define PE(n)    GPIOPort::E(n)
    //! Gets a GPIOPinID for the specified pin on port E
    #define pE(n)    GPIOPinID(4, (n))
#endif

#if _GPIO_PORT_F_PIN_COUNT
    //! Gets a GPIOPin instance for the specified pin on port F
    static constexpr GPIOPin F(int pin) { return GPIOPin(GPIO_P(5), BIT(pin)); }
    //! Gets a GPIOPin instance for the specified pin on port F
    #define PF(n)    GPIOPort::F(n)
    //! Gets a GPIOPinID for the specified pin on port F
    #define pF(n)    GPIOPinID(5, (n))
#endif

#if _GPIO_PORT_G_PIN_COUNT
    //! Gets a GPIOPin instance for the specified pin on port G
    static constexpr GPIOPin G(int pin) { return GPIOPin(GPIO_P(6), BIT(pin)); }
    //! Gets a GPIOPin instance for the specified pin on port G
    #define PG(n)    GPIOPort::G(n)
    //! Gets a GPIOPinID for the specified pin on port G
    #define pG(n)    GPIOPinID(6, (n))
#endif

#if _GPIO_PORT_H_PIN_COUNT
    //! Gets a GPIOPin instance for the specified pin on port H
    static constexpr GPIOPin H(int pin) { return GPIOPin(GPIO_P(7), BIT(pin)); }
    //! Gets a GPIOPin instance for the specified pin on port H
    #define PH(n)    GPIOPort::H(n)
    //! Gets a GPIOPinID for the specified pin on port H
    #define pH(n)    GPIOPinID(7, (n))
#endif

#if _GPIO_PORT_I_PIN_COUNT
    //! Gets a GPIOPin instance for the specified pin on port I
    static constexpr GPIOPin I(int pin) { return GPIOPin(GPIO_P(8), BIT(pin)); }
    //! Gets a GPIOPin instance for the specified pin on port I
    #define PI(n)    GPIOPort::I(n)
    //! Gets a GPIOPinID for the specified pin on port I
    #define pI(n)    GPIOPinID(8, (n))
#endif

    //! Gets a GPIOPin instance for the specified pin on the GPIOPort
    GPIOPin Pin(int number) { return GPIOPin(this, BIT(number)); }
    //! Gets the zero-based index of the GPIOPort
    int Index() const { return GetIndex(this); }

#ifdef _GPIO_P_CTRL_SLEWRATE_MASK
    //! Configures the drive options for the GPIOPort
    /*!
     * The port is configured automatically to @ref EFM32_GPIO_DRIVE_DEFAULT
     * when configuring the first pin
     */
    void Setup(uint32_t drive = EFM32_GPIO_DRIVE_DEFAULT)
    {
#ifdef _GPIO_P_CTRL_DRIVESTRENGTH_MASK
        DBGCL("gpio", "Configuring port %c drive %s(%d), ALT %s(%d)",
            'A' + Index(),
            (drive & _GPIO_P_CTRL_DRIVESTRENGTH_MASK) == GPIO_P_CTRL_DRIVESTRENGTH_WEAK ? "WEAK" : "STRONG",
            (drive & _GPIO_P_CTRL_SLEWRATE_MASK) >> _GPIO_P_CTRL_SLEWRATE_SHIFT,
            (drive & _GPIO_P_CTRL_DRIVESTRENGTHALT_MASK) == GPIO_P_CTRL_DRIVESTRENGTHALT_WEAK ? "WEAK" : "STRONG",
            (drive & _GPIO_P_CTRL_SLEWRATEALT_MASK) >> _GPIO_P_CTRL_SLEWRATEALT_SHIFT);
#else
        DBGCL("gpio", "Configuring port %c drive %d, ALT %d",
            'A' + Index(),
            (drive & _GPIO_P_CTRL_SLEWRATE_MASK) >> _GPIO_P_CTRL_SLEWRATE_SHIFT,
            (drive & _GPIO_P_CTRL_SLEWRATEALT_MASK) >> _GPIO_P_CTRL_SLEWRATEALT_SHIFT);
#endif
        CTRL = drive;
    }
#endif

    static constexpr int GetIndex(const GPIOPort* port) { return (((uintptr_t)port) - GPIO_BASE) / sizeof(GPIOPort); }

#ifdef EFM32_PERIPHERAL_BITMOD
    volatile uint32_t* BitSetPtr() { return EFM32_BITMODPTR(true, &DOUT); }
    volatile uint32_t* BitClearPtr() { return EFM32_BITMODPTR(false, &DOUT); }
#endif
#ifdef EFM32_PERIPHERAL_BITTGL
    volatile uint32_t* TogglePtr() { return EFM32_BITTGLPTR(&DOUT); }
#else
    volatile uint32_t* TogglePtr() { return &DOUTTGL; }
#endif
    const volatile uint32_t* InputPtr() { return &DIN; }
    volatile uint32_t* OutputPtr() { return &DOUT; }

    //! Suppress trace messages when reconfiguring pins
    static void SuppressConfigTrace() { _Trace(1); }
    //! Restore trace messages when reconfiguring pins
    static void RestoreConfigTrace() { _Trace(-1); }

private:
    struct AltSpec
    {
        constexpr AltSpec(unsigned pin, GPIOPin::Mode mode, uint8_t route, uint8_t loc)
            : spec(pin | mode << 8 | route << 16 | loc << 24) {}

        union
        {
            uint32_t spec;
            struct
            {
                uint8_t pin;
                GPIOPin::Mode mode;
                uint8_t route, loc;
            };
        };
    };

    void Configure(uint32_t mask, enum GPIOPin::Mode mode);
#ifdef EFM32_GPIO_LINEAR_INDEX
    void ConfigureAlternate(AltSpec spec, volatile uint32_t& routepen, unsigned locOffset);
#endif
#if defined(_SILICON_LABS_32B_SERIES_1)
    void ConfigureAlternate(AltSpec spec, volatile uint32_t& routepen, GPIOLocations_t locations);
#elif defined(_SILICON_LABS_32B_SERIES_2)
    void ConfigureAlternate(AltSpec spec, volatile uint32_t& routeen);
#endif
#ifdef Ckernel
    async(WaitFor, uint32_t indexAndState, Timeout timeout = Timeout::Infinite);
#endif

#if TRACE
    static bool _Trace(int n = 0) { static int noTrace = 0; return !(noTrace += n); }
#else
    static constexpr bool _Trace(int n = 0) { return false; }
#endif

    friend class GPIOPin;
    friend class GPIOBlock;
};

//! Represents the entire GPIO peripheral
class GPIOBlock : public GPIO_TypeDef
{
private:
    uint32_t EnableInterrupt(GPIOPinID id, unsigned risingFalling);
    void DisableInterrupt(uint32_t mask);

    void ConfigureWakeFromEM4(GPIOPinID id, bool level);

    ALWAYS_INLINE void EnableTrace()
    {
#ifdef _SILICON_LABS_32B_SERIES_1
    // enable default SWO pin (PF2)
    MODMASK(P[5].MODEL, _GPIO_P_MODEL_MODE2_MASK, GPIO_P_MODEL_MODE2_PUSHPULL);
    ROUTEPEN |= GPIO_ROUTEPEN_SWVPEN;
#if EFM32_GPIO_DRIVE_CONTROL
    P[5].CTRL = EFM32_GPIO_DRIVE_DEFAULT;
#endif

    // enable AUXHFRCO
    CMU->EnableAUXHFRCO();
    while (!CMU->AUXHFRCOReady());
#elif defined(_SILICON_LABS_32B_SERIES_2)
    // enable watchdog
    CMU->EnableWDOG(0);
    // enable default SWO pin (PA3)
    TRACEROUTEPEN_SET = GPIO_TRACEROUTEPEN_SWVPEN;
#if EFM32_GPIO_DRIVE_CONTROL
    P[0].CTRL = EFM32_GPIO_DRIVE_DEFAULT;
#endif
    MODMASK(P[0].MODEL, _GPIO_P_MODEL_MODE3_MASK, GPIO_P_MODEL_MODE3_PUSHPULL);
#endif
    }

    friend class GPIOPin;
    friend class GPIOPort;
    friend void _efm32_startup();
};

DEFINE_FLAG_ENUM(enum GPIOPin::Mode);

class GPIOPin;

//! Represents an invalid/unused GPIO pin - it still gets a valid port, but no mask
#define Px       GPIOPin((GPIOPort*)&GPIO->P[0], 0)

ALWAYS_INLINE constexpr GPIOPinID GPIOPin::GetID() const { return GPIOPinID(GPIOPort::GetIndex(port), Index()); }

ALWAYS_INLINE void GPIOPin::Configure(Mode mode) const { port->Configure(mask, mode); }
#if defined(_SILICON_LABS_32B_SERIES_1)
ALWAYS_INLINE void GPIOPin::ConfigureAlternate(Mode mode, volatile uint32_t& routepen, uint8_t route, uint8_t locIndex, GPIOLocations_t locations) const { port->ConfigureAlternate(GPIOPort::AltSpec(Index(), mode, route, locIndex), routepen, locations); }
#elif defined(_SILICON_LABS_32B_SERIES_2)
ALWAYS_INLINE void GPIOPin::ConfigureAlternate(Mode mode, volatile uint32_t& routeen, unsigned penBit, unsigned routeOffset) const { port->ConfigureAlternate(GPIOPort::AltSpec(Index(), mode, penBit, routeOffset), routeen); }
#endif
#ifdef EFM32_GPIO_LINEAR_INDEX
ALWAYS_INLINE constexpr unsigned GPIOPin::GetLinearIndex(unsigned offset) const { return (Index() - offset + EFM32_GPIO_LINEAR_INDEX[GPIOPort::GetIndex(port)]) & 31; }
ALWAYS_INLINE void GPIOPin::ConfigureAlternate(Mode mode, volatile uint32_t& routepen, uint8_t route, uint8_t locIndex, unsigned locOffset) const { port->ConfigureAlternate(GPIOPort::AltSpec(Index(), mode, route, locIndex), routepen, locOffset); }
#endif
ALWAYS_INLINE uint32_t GPIOPin::EnableInterrupt(bool rising, bool falling) const { return GPIO->EnableInterrupt(GetID(), (rising * 1) | (falling * 2)); }
#ifdef Ckernel
ALWAYS_INLINE async(GPIOPin::WaitFor, bool state, Timeout timeout) { return async_forward(Port().WaitFor, (state << 4) | Index(), timeout); }
#endif
ALWAYS_INLINE void GPIOPin::ConfigureWakeFromEM4(bool level) const
{
    // configure input with filtering and pull away from the desired wakeup level
    Configure(Mode(InputPull | FlagFilter | (!level * FlagSet)));
    GPIO->ConfigureWakeFromEM4(GetID(), level);
}

ALWAYS_INLINE void GPIOPin::Set() const { EFM32_BITSET(port->DOUT, mask); }
ALWAYS_INLINE void GPIOPin::Res() const { EFM32_BITCLR(port->DOUT, mask); }
#ifdef EFM32_PERIPHERAL_BITTGL
ALWAYS_INLINE void GPIOPin::Toggle() const { EFM32_BITTGL(port->DOUT, mask); }
#else
ALWAYS_INLINE void GPIOPin::Toggle() const { port->DOUTTGL = mask; }
#endif
ALWAYS_INLINE void GPIOPin::Set(bool state) const { EFM32_BITMOD(state, port->DOUT, mask); }
ALWAYS_INLINE bool GPIOPin::Get() const { return port->DIN & mask; }

class APORTX : public APORT
{
public:
    constexpr APORTX(Fixed f) : APORT(f) {}
    constexpr APORTX(APORT other) : APORT(other) {}
#if EFM32_GPIO_APORT_AVAILABLE
    constexpr APORTX(const GPIOPin& pin) : APORT(pin.GetID().APORTX()){}
    ALWAYS_INLINE constexpr APORTX(const GPIOPinID pin) : APORT(pin.APORTX()) {}
#endif
};

class APORTY : public APORT
{
public:
    constexpr APORTY(Fixed f) : APORT(f) {}
    constexpr APORTY(APORT other) : APORT(other) {}
#if EFM32_GPIO_APORT_AVAILABLE
    constexpr APORTY(const GPIOPin& pin) : APORT(pin.GetID().APORTY()) {}
    ALWAYS_INLINE constexpr APORTY(const GPIOPinID pin) : APORT(pin.APORTY()) {}
#endif
};
