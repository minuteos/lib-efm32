/*
 * Copyright (c) 2020 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32/boot-spiflash/plugin.cpp
 */

#include <kernel/kernel.h>

#include <hw/GPIO.h>
#include <hw/EMU.h>

#include <boot/gecko_bootloader.h>

#include <storage/SPIFlash.h>

using namespace kernel;
using namespace storage;

BEGIN_EXTERN_C

extern "C" const BootloaderStorageLayout_t storageLayout =
{
    SPIFLASH,
    BTL_PLUGIN_STORAGE_NUM_SLOTS,
    BTL_PLUGIN_STORAGE_SLOTS
};

BootloaderStorageImplementationInformation_t deviceInfo =
{
    .version = BOOTLOADER_STORAGE_IMPL_INFO_VERSION,
    .capabilitiesMask = BOOTLOADER_STORAGE_IMPL_CAPABILITY_ERASE_SUPPORTED | BOOTLOADER_STORAGE_IMPL_CAPABILITY_PAGE_ERASE_REQUIRED,
    .pageEraseMs = 5000,
    .partEraseMs = 500000,
    .pageSize = 0,
    .partSize = 0,
    .partDescription = "SPIFlash",
    .wordSizeBytes = 1,
};

BootloaderStorageImplementationInformation_t* getDeviceInfo()
{
    return &deviceInfo;
}

#if EFM32_USE_GPIO_SPI
bus::SPI_GPIO spi(SPI_CLK, SPI_MOSI, SPI_MISO);
#else
#define spi SPI_BUS
#endif

struct Runner
{
    Scheduler s;
    SPIFlash flash;
    uint32_t addr;
    void* data;
    size_t length;
    intptr_t result;

    Runner() : flash(spi, SPI_CS_FLASH) {}

    async(Init) { auto res = async_forward(flash.Init); result = _ASYNC_RES_VALUE(res); return res; }
    async(Read) { return async_forward(flash.Read, addr, Buffer(data, length)); }
    async(Write) { return async_forward(flash.Write, addr, Span(data, length)); }
    async(Erase) { return async_forward(flash.Erase, addr, length); }

    bool Init()
    {
        s.Add(this, &Runner::Init);
        s.Run();
        return result;
    }

    void Read(uint32_t addr, void* data, size_t length)
    {
        this->addr = addr;
        this->data = data;
        this->length = length;
        s.Add(this, &Runner::Read);
        s.Run();
    }

    void Write(uint32_t addr, void* data, size_t length)
    {
        this->addr = addr;
        this->data = data;
        this->length = length;
        s.Add(this, &Runner::Write);
        s.Run();
    }

    void Erase(uint32_t addr, size_t length)
    {
        this->addr = addr;
        this->length = length;
        s.Add(this, &Runner::Erase);
        s.Run();
    }

    void Sync()
    {
        s.Add(&flash, &SPIFlash::Sync);
        s.Run();
    }
};

static Runner runner;

int32_t storage_init()
{
    EMU->Configure();

#ifdef GECKO_BOOTLOADER_CUSTOM_STORAGE_INIT
    GECKO_BOOTLOADER_CUSTOM_STORAGE_INIT();
#endif

    SPI_MISO.ConfigureDigitalInput();
    SPI_MOSI.ConfigureDigitalOutput();
    SPI_CLK.ConfigureDigitalOutput();
    SPI_CS_FLASH.ConfigureDigitalOutput(true);

#if !EFM32_USE_GPIO_SPI
    SPI_BUS->EnableClock();
    SPI_BUS->Setup(USART::ModeSynchronous | USART::ClockIdleLow | USART::AutoChipSelect | USART::PhaseSampleLeading | USART::TxInterruptLevelHalfFull | USART::OrderMSBFirst);
    SPI_BUS->OutputFrequency(4000000);
    SPI_BUS->ConfigureRx(SPI_MISO);
    SPI_BUS->ConfigureTx(SPI_MOSI);
    SPI_BUS->ConfigureClk(SPI_CLK);

    SPI_BUS->TxEnable();
    SPI_BUS->RxEnable();
    SPI_BUS->MasterEnable();
#endif

    if (!runner.Init())
    {
        return BOOTLOADER_ERROR_INIT_STORAGE;
    }

    deviceInfo.pageSize = runner.flash.SectorSize();
    deviceInfo.partSize = runner.flash.Size();

    return BOOTLOADER_OK;
}

int32_t storage_shutdown()
{
#if !EFM32_USE_GPIO_SPI
    SPI_BUS->DisableClock();
#endif

    SPI_MISO.Disable();
    SPI_MOSI.Disable();
    SPI_CLK.Disable();
    SPI_CS_FLASH.Disable();

#ifdef GECKO_BOOTLOADER_CUSTOM_STORAGE_SHUTDOWN
    GECKO_BOOTLOADER_CUSTOM_STORAGE_SHUTDOWN();
#endif

    return BOOTLOADER_OK;
}

int32_t storage_readRaw(uint32_t address, uint8_t *data, size_t length)
{
    runner.Read(address, data, length);
    return BOOTLOADER_OK;
}

int32_t storage_writeRaw(uint32_t address, uint8_t *data, size_t length)
{
    runner.Write(address, data, length);
    return BOOTLOADER_OK;
}

int32_t storage_eraseRaw(uint32_t address, size_t totalLength)
{
    runner.Erase(address, totalLength);
    return BOOTLOADER_OK;
}

bool storage_isBusy()
{
    runner.Sync();
    return false;
}

END_EXTERN_C
