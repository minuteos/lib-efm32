/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-usb/usb/Descriptors.h
 * 
 * Standard USB descriptors
 */

#pragma once

#include <base/base.h>

#include <tuple>

namespace usb
{

//! Standard USB Descriptor types
enum struct DescriptorType : uint8_t
{
    Device = 1,     //!< DeviceDescriptor
    Config = 2,     //!< ConfigDescriptorHeader
    String = 3,     //!< StringDescriptor
    Interface = 4,  //!< InterfaceDescriptorHeader
    Endpoint = 5,   //!< EndpointDescriptor

    ClassSpecific = 0x20,   //!< Class-specific descriptor flag
    ClassSpecificDevice = ClassSpecific | Device,   //!< Class-specific device descriptor
    ClassSpecificConfig = ClassSpecific | Config,   //!< Class-specific configuration descriptor
    ClassSpecificInterface = ClassSpecific | Interface, //!< Class-specific interface descriptor
    ClassSpecificEndpoint = ClassSpecific | Endpoint, //!< Class-specific endpoint descriptor
};

//! Subtypes for class-specific descriptors
enum struct DescriptorSubType : uint8_t
{
    CdcHeader = 0,  //!< Header Functional Descriptor
    CdcCallManagement = 1, //!< Call Management Functional Descriptor
    CdcAcm = 2,     //!< Abstracto Control Management Functional Descriptor
    CdcDlm = 3,     //!< Direct Line Management Functional Descriptor
    CdcRing = 4,    //!< Telephone Ringer Functional Descriptor
    CdcCall = 5,    //!< Telephone Call and Line State Reporting Capabilities Functional Descriptor
    CdcUnion = 6,   //!< Union Functional Descriptor
    CdcCountry = 7, //!< Country Selection Functional Descriptor
    CdcOpMode = 8,  //!< Telephone Operational Modes Functional Descriptor
    CdcTerminal = 9,    //!< USB Terminal Functional Descriptor
    CdcNetTerm = 10,    //!< Network Channel Terminal Descriptor
    CdcProtoUnit = 11,  //!< Protocol Unit Functional Descriptor
    CdcExtUnit = 12,    //!< Extension Unit Functional Descriptor
    CdcMcm = 13,        //!< Multi-Channel Management Functional Descriptor
    CdcCapi = 14,       //!< CAPI Control Management Functional Descriptor
    CdcEthernet = 15,   //!< Ethernet Networking Functional Descriptor
    CdcAtm = 16,        //!< ATM Networking Functional Descriptor
    CdcWhc = 17,        //!< Wireless Handset Control Model Functional Descriptor
    CdcMdlm = 18,        //!< Mobile Direct Line Model Functional Descriptor
    CdcMdlDetail = 19,  //! MDLM Detail Functional Descriptor
    CdcDevManagement = 20,  //!< Device Management Model Functional Descriptor
    CdcObex = 21,       //!< OBEX Functional Descriptor
    CdcCmdSet = 22,     //!< Command Set Functional Descriptor
    CdcCmdDetail = 23,  //!< Command Set Detail Functional Descriptor
    CdcTcm = 24,        //!< Telephone Control Model Functional Descriptor
    CdcObexId = 25,     //!< OBEX Service Identifier Functional Descriptor
    CdcNcm = 26,        //!< NCM Functional Descriptor
};

//! ConfigDescriptor attributes
enum struct ConfigAttributes : uint8_t
{
    BusPowered = 0,             //!< Device is powered by the bus (default)
    RemoteWakeup = BIT(5),      //!< Device can wake the host
    SelfPowered = BIT(6),       //!< Device is selfpowered
    _Reserved1 = BIT(7),        //!< Reserved bit, must be 1
};

//! Endpoint type
enum struct EndpointType
{
    Control = 0,        //!< Control Endpoint type
    Isochronous = 1,    //!< Isochronous Endpoint type
    Bulk = 2,           //!< Bulk Endpoint type
    Interrupt = 3,      //!< Interrupt Endpoint type
};

//! Isochronous endpoint synchronization mode
enum struct IsoSync : uint8_t
{
    None = 0,
    Asynchronous = 1,
    Adaptive = 2,
    Synchronous = 3,
};

//! Isochronous endpoint usage
enum struct IsoUsage : uint8_t
{
    Data = 0,
    Feedback = 1,
    ExplicitFeedback = 2,
};

DEFINE_FLAG_ENUM(ConfigAttributes);

//! Standard USB device classes
enum struct DeviceClass : uint8_t
{
    None = 0,       //!< Class specified per-interface
    Cdc = 2,        //!< Communications device class
    Billboard = 17, //!< Billboard device class
    Diag = 0xDC,    //!< Diagnostic device class
    Misc = 0xEF,    //!< Miscellaneous device class
    Vendor = 0xFF,  //!< Vendor-specific device class
};

//! Standard USB interface classes
enum struct InterfaceClass : uint8_t
{
    None = 0,       //!< Class specified in DeviceDescriptor
    Audio = 1,      //!< Audio interface class
    Cdc = 2,        //!< Communications device control interface class
    Hid = 3,        //!< Human interface device class
    Physical = 5,   //!< Physical interface class
    Image = 6,      //!< Still imaging interface class
    Printer = 7,    //!< Printer interface class
    MassStorage = 8,    //!< Mass storage interface class
    CdcData = 10,   //!< Communications device data interface class
    SmartCard = 11, //!< Smart card interface class
    Content = 13,   //!< Content security interface class
    Video = 14,     //!< Video interface class
    Healthcare = 15,    //!< Personal healthcare interface class
    AudioVideo = 16,    //!< Audio/Video device interface class
    Diag = 0xDC,    //!< Diagnostic interface class
    Wireless = 0xE0,    //!< Wireless controller interface class
    Misc = 0xEF,    //!< Miscellaneous interface class
    App = 0xFE,     //!< Application-specific interface class
    Vendor = 0xFF,  //!< Vendor-specific interface class
};

//! Standard USB subclasses
//! Values are prefixed by the name of the DeviceClass or InterfaceClass with which they are associated
enum struct SubClass : uint8_t
{
    None = 0,       //!< Subclass not specified

    // Cdc Subclasses follow
    CdcDlc = 1,     //!< Direct Line Control Model
    CdcAcm = 2,     //!< Abstract Control Model
    CdcTcm = 3,     //!< Telephone Control Model
    CdcMcc = 4,     //!< Multi-channel Control Model
    CdcCapi = 5,    //!< CAPI Control Model
    CdcEnc = 6,     //!< Ethernet Networking Control Model
    CdcAtm = 7,     //!< ATM Networking Control Model
    CdcWhc = 8,     //!< Wireless Handset Control Model
    CdcDevMgmt = 9, //!< Device Management
    CdcMdlc = 10,   //!< Mobile Direct Line Control Model
    CdcObex = 11,   //!< OBEX
    CdcEem = 12,    //!< Ethernet Emulation Model
    CdcNcm = 13,    //!< Network Control Model

    Vendor = 0xFF,  //!< Vendor-specific subclass
};

//! Standard USB protocols
//! Values are prefixed by the name of the SubClass with which they are associated
enum struct Protocol : uint8_t
{
    None = 0,       //!< Protocol not specified

    // Cdc Protocols follow
    CdcAt = 1,      //!< AT Commands V.250 etc.
    CdcPcca101 = 2, //!< AT Commands defined by PCCA-101
    CdcPcca101O = 3,    //!< AT Commands defined by PCCA-101 & Annex O
    CdcGsm = 4,     //!< AT Commands defined by GSM 07.07
    Cdc3Gpp = 5,    //!< AT Commands defined by 3GPP 27.007
    CdcCdma = 6,    //!< AT Commands defined by TIA for CDMA
    CdcEem = 7,     //!< Ethernet Emulation Model

    Vendor = 0xFF,  //!< Vendor-specific protocol
};

//! Common header of all descriptors, can be used to walk consecutive descriptors
PACKED_UNALIGNED_STRUCT DescriptorHeader
{
    constexpr DescriptorHeader(uint8_t size, DescriptorType type)
        : bLength(size), bDescriptorType(type) {}

    uint8_t bLength;                //!< Length of the descriptor in bytes
    DescriptorType bDescriptorType; //!< Type of the descriptor

    //! Gets the next consecutive descriptor
    const DescriptorHeader* Next() const { return (const DescriptorHeader*)((uintptr_t)this + bLength); }
};

//! Defines the attributes of the physical device, such as global class, vendor and product ID, etc.
PACKED_UNALIGNED_STRUCT DeviceDescriptor : DescriptorHeader
{
    constexpr DeviceDescriptor(DeviceClass cls, SubClass subCls, Protocol proto, uint16_t vid, uint16_t pid, uint16_t ver,
        uint8_t strMfg = 0, uint8_t strProduct = 0, uint8_t strSerial = 0, uint8_t numConfig = 1) :
        DescriptorHeader(sizeof(DeviceDescriptor), DescriptorType::Device),
        bDeviceClass(cls),
        bDeviceSubClass(subCls),
        bDeviceProtocol(proto),
        idVendor(vid),
        idProduct(pid),
        bcdDevice(ver),
        iManufacturer(strMfg),
        iProduct(strProduct),
        iSerialNumber(strSerial),
        bNumConfigurations(numConfig) {}

    uint16_t bcdUSB = 0x0200;                       //!< USB specification version (BCD)
    DeviceClass bDeviceClass;                       //!< DeviceClass
    SubClass bDeviceSubClass;                       //!< Device SubClass
    Protocol bDeviceProtocol;                       //!< Device Protocol
    uint8_t bMaxPacketSize0 = 64;                   //!< Maximum packet size for control endpoint
    uint16_t idVendor;                              //!< Vendor ID
    uint16_t idProduct;                             //!< Product ID
    uint16_t bcdDevice;                             //!< Device version (BCD)
    uint8_t iManufacturer;                          //!< Manufacturer name string index
    uint8_t iProduct;                               //!< Product name string index
    uint8_t iSerialNumber;                          //!< Product serial number string index
    uint8_t bNumConfigurations;                     //!< Number of configurations
};

typedef int _Empty[0];

template<typename T1 = _Empty, typename T2 = _Empty, typename... TRest> PACKED_UNALIGNED_STRUCT ConfigChildren
{
    constexpr ConfigChildren(const T1& first, const T2& second, const TRest&... rest)
        : first(first), rest(second, rest...) {}

    T1 first;
    ConfigChildren<T2, TRest...> rest;
};

template<typename T1> PACKED_UNALIGNED_STRUCT ConfigChildren<T1, _Empty>
{
    constexpr ConfigChildren(const T1& first)
        : first(first) {}

    T1 first;
};

//! Configuration definition header, immediately followed by interface descriptors
PACKED_UNALIGNED_STRUCT ConfigDescriptorHeader : DescriptorHeader
{
    constexpr ConfigDescriptorHeader(uint16_t interfacesSize, uint8_t numInterfaces, uint8_t index, int maxPower, uint8_t strName, ConfigAttributes attributes)
        : DescriptorHeader(sizeof(ConfigDescriptorHeader), DescriptorType::Config),
        wTotalLength(sizeof(ConfigDescriptorHeader) + interfacesSize),
        bNumInterfaces(numInterfaces),
        bConfigurationValue(index),
        iConfiguration(strName),
        bmAttributes(attributes | ConfigAttributes::_Reserved1),
        bMaxPower(maxPower / 2) {}

    constexpr const DescriptorHeader* End() const { return (const DescriptorHeader*)((uintptr_t)this + wTotalLength); }
    const class EndpointDescriptor* FindEndpoint(uint8_t address, int interface = -1, int alternate = 0) const;

    uint16_t wTotalLength;          //!< Total length including all nested descriptors
    uint8_t bNumInterfaces;         //!< Number of nested interface descriptors
    uint8_t bConfigurationValue;    //!< Selector value for this configuration
    uint8_t iConfiguration;         //!< Configuration name string index
    ConfigAttributes bmAttributes;  //!< Configuration attributes
    uint8_t bMaxPower;              //!< Maximum power draw in 2 mA units
};

//! Full configuration definition, with interface descriptors embedded
template<typename... TInterfaces> PACKED_UNALIGNED_STRUCT ConfigDescriptorBlock : ConfigDescriptorHeader
{
    constexpr ConfigDescriptorBlock(uint8_t index, int maxPower, uint8_t strName, ConfigAttributes attributes, const TInterfaces&... interfaces) :
        ConfigDescriptorHeader(sizeof(ConfigChildren<TInterfaces...>), sizeof...(TInterfaces), index, maxPower, strName, attributes),
        interfaces(interfaces...) {}

    ConfigChildren<TInterfaces...> interfaces;  //!< Nested interface definitions
};

//! Creates a ConfigDescriptorHeader with the specified parameters
constexpr ConfigDescriptorHeader ConfigDescriptor(uint8_t index, int maxPower, uint8_t strName, ConfigAttributes attributes)
{
    return ConfigDescriptorHeader(0, 0, index, maxPower, strName, attributes);
}

//! Creates a ConfigDescriptorBlock with the specified parameters and nested interfaces
template<typename... TInterfaces> constexpr ConfigDescriptorBlock<TInterfaces...> ConfigDescriptor(uint8_t index, int maxPower, uint8_t strName, ConfigAttributes attributes, const TInterfaces&... interfaces)
{
    return ConfigDescriptorBlock<TInterfaces...>(index, maxPower, strName, attributes, interfaces...);
}

//! Interface definition header, immediately followed by endpoint descriptors
PACKED_UNALIGNED_STRUCT InterfaceDescriptorHeader : DescriptorHeader
{  
    constexpr InterfaceDescriptorHeader(uint8_t index, uint8_t alternate, uint8_t numEndpoints, InterfaceClass cls, SubClass subCls, Protocol proto, uint8_t strName = 0)
        : DescriptorHeader(sizeof(InterfaceDescriptorHeader), DescriptorType::Interface),
        bInterfaceNumber(index),
        bAlternateSetting(alternate),
        bNumEndpoints(numEndpoints),
        bInterfaceClass(cls),
        bInterfaceSubClass(subCls),
        bInterfaceProtocol(proto),
        iInterface(strName) {}

    uint8_t bInterfaceNumber;       //!< Number of the interface
    uint8_t bAlternateSetting;      //!< Number of interface alternate setting
    uint8_t bNumEndpoints;          //!< Number of endpoints in this interface
    InterfaceClass bInterfaceClass; //!< Interface Class
    SubClass bInterfaceSubClass;    //!< Interface SubClass
    Protocol bInterfaceProtocol;    //!< Interface Protocol
    uint8_t iInterface;             //!< Interface name string index
};

template<typename T> constexpr int is_endpoint() { return 0; }
template<> constexpr int is_endpoint<class EndpointDescriptor>() { return 1; }
template<typename T1 = _Empty, typename... TRest> static constexpr int endpoint_count() { return is_endpoint<T1>() + endpoint_count<TRest...>(); }
template<> constexpr int endpoint_count<_Empty>() { return 0; }

//! Full interface definition, with endpoint and other descriptors embedded
template<typename... TEndpoints> PACKED_UNALIGNED_STRUCT InterfaceDescriptorBlock : InterfaceDescriptorHeader
{
    constexpr InterfaceDescriptorBlock(uint8_t index, uint8_t alternate, InterfaceClass cls, SubClass subCls, Protocol proto, uint8_t strName, const TEndpoints&... endpoints) :
        InterfaceDescriptorHeader(index, alternate, endpoint_count<TEndpoints...>(), cls, subCls, proto, strName),
        endpoints(endpoints...) {}

    ConfigChildren<TEndpoints...> endpoints;
};

//! Creates an InterfaceDescriptorHeader with the specified parameters
constexpr InterfaceDescriptorHeader InterfaceDescriptor(uint8_t index, uint8_t alternate, InterfaceClass cls, SubClass subCls, Protocol proto, uint8_t strName = 0)
{
    return InterfaceDescriptorHeader(index, alternate, 0, cls, subCls, proto, strName);
}

//! Creates an InterfaceDescriptorBlock with the specified parameters and nested descriptors
template<typename... TEndpoints> constexpr InterfaceDescriptorBlock<TEndpoints...> InterfaceDescriptor(uint8_t index, uint8_t alternate, InterfaceClass cls, SubClass subCls, Protocol proto, uint8_t strName = 0, const TEndpoints&... endpoints)
{
    return InterfaceDescriptorBlock<TEndpoints...>(index, alternate, cls, subCls, proto, strName, endpoints...);
}

//! Defines an interface endpoint
PACKED_UNALIGNED_STRUCT EndpointDescriptor
{
    constexpr EndpointDescriptor(uint8_t address, uint8_t attributes, uint16_t maxPacketSize, uint8_t interval) :
        bEndpointAddress(address),
        bmAttributes(attributes),
        wMaxPacketSize(maxPacketSize),
        bInterval(interval) {}

    constexpr EndpointDescriptor(bool in, uint8_t number, EndpointType type, uint16_t maxPacketSize, uint8_t interval = 1, IsoSync sync = IsoSync::None, IsoUsage usage = IsoUsage::Data) :
        number(number),
        in(in),
        type((uint8_t)type),
        isoSync((uint8_t)sync),
        isoUsage((uint8_t)usage),
        wMaxPacketSize(maxPacketSize),
        bInterval(interval) {}

    static constexpr EndpointDescriptor ControlIn()
    { return EndpointDescriptor(true, 0, EndpointType::Control, 64); }
    static constexpr EndpointDescriptor ControlOut()
    { return EndpointDescriptor(false, 0, EndpointType::Control, 64); }

    static constexpr EndpointDescriptor BulkIn(uint8_t number, uint16_t maxPacketSize)
    { return EndpointDescriptor(true, number, EndpointType::Bulk, maxPacketSize); }
    static constexpr EndpointDescriptor BulkOut(uint8_t number, uint16_t maxPacketSize)
    { return EndpointDescriptor(false, number, EndpointType::Bulk, maxPacketSize); }

    static constexpr EndpointDescriptor InterruptIn(uint8_t number, uint16_t maxPacketSize, uint8_t pollInterval)
    { return EndpointDescriptor(true, number, EndpointType::Interrupt, maxPacketSize, pollInterval); }
    static constexpr EndpointDescriptor InterruptOut(uint8_t number, uint16_t maxPacketSize, uint8_t pollInterval)
    { return EndpointDescriptor(true, number, EndpointType::Interrupt, maxPacketSize, pollInterval); }

    uint8_t bLength = sizeof(EndpointDescriptor);   //!< Length of the EndpointDescriptor
    DescriptorType bDescriptorType = DescriptorType::Endpoint;  //!< EndpointDescriptor type
    union
    {
        uint8_t bEndpointAddress;       //!< Address of the endpoint (including direction in the top bit)
        struct
        {
            uint8_t number : 4;         //!< Number of the endpoint
            uint8_t : 3;
            uint8_t in : 1;             //!< Direction of the endpoint
        };
    };
    union
    {
        uint8_t bmAttributes;           //!< Attributes of the endpoint
        struct
        {
            uint8_t type : 2;           //!< Endpoint type
            uint8_t isoSync : 2;        //!< Isochronous endpont synchronization type
            uint8_t isoUsage : 2;       //!< Isochronous endpoint usage
            uint8_t : 2;
        };
    };
    uint16_t wMaxPacketSize;            //!< Endpoint maximum packet size in bytes
    uint8_t bInterval;                  //!< Endpoing polling interval
};

//! Defines a custom descriptor (used for class- and vendor- specific descriptors)
template<typename... TContent> PACKED_UNALIGNED_STRUCT CustomDescriptor
{
    constexpr CustomDescriptor(DescriptorType type, const TContent&... content) :
        bDescriptorType(type),
        content(content...) {}

    uint8_t bLength = sizeof(CustomDescriptor); //!< Length of the CustomDescriptor
    DescriptorType bDescriptorType;
    ConfigChildren<TContent...> content;
};

template<typename... TContent> constexpr auto ClassSpecificInterfaceDescriptor(DescriptorSubType subType, const TContent&... content)
{
    return CustomDescriptor<DescriptorSubType, TContent...>(DescriptorType::ClassSpecificInterface, subType, content...);
}

//! Defines a static UTF16LE encoded string
PACKED_UNALIGNED_STRUCT StringDescriptor
{
    uint8_t len;          //!< Length of the StringDescriptor including the content
    DescriptorType type;  //!< StringDescriptor type
    char16_t value[];     //!< StringDescriptor content

    //! Gets the next string descriptor in the table
    const StringDescriptor* Next() const
    {
        // moves to the next descriptor
        // skips an additional two bytes, since the strings are actually null-terminated
        // also word-aligns up, as each string descriptor starts at a word boundary
        return (const StringDescriptor*)(((uintptr_t)this + len + 2 + 3) & ~3); 
    }
};

// descriptors must be aligned, the USB controller won't be able to send them otherwise
template<size_t n> PACKED_UNALIGNED_STRUCT _StringDescriptor
{
    uint8_t len;          //!< Length of the StringDescriptor including the content, minus null terminator
    DescriptorType type;  //!< StringDescriptor type
    const char16_t value[n];    //!< StringDescriptor content
};

}

#define USB_STRING_TABLE_START(...) \
const struct UNIQUE(UsbStringTable) { \
    static constexpr int __count0 = __COUNTER__; \
    __attribute__((aligned(4))) const ::usb::_StringDescriptor<1> _languages = { 2, ::usb::DescriptorType::String, { __VA_ARGS__ } };

#define USB_STRING(name, value) \
    static constexpr uint8_t name = __COUNTER__ - __count0; \
    __attribute__((aligned(4))) const ::usb::_StringDescriptor<countof(value)> name ## _desc = { countof(value) * 2, ::usb::DescriptorType::String, value };

#define USB_STRING_TABLE_END(tableName) \
    static constexpr int count = __COUNTER__ - __count0; \
    __attribute__((aligned(4))) const ::usb::StringDescriptor _terminator = { }; \
    constexpr operator const usb::StringDescriptor*() const { return (const usb::StringDescriptor*)&_languages; }; \
} tableName;
