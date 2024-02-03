#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "drivers/eeprom.h"
#include "drivers/eeprom_impl.h"
#include "drivers/eeprom_at24c02.h"
#include "drivers/bus_i2c.h"

#define AT24C02_I2C_ADDRESS         0x50

#define AT24C02_MAGICNUMBER         0x524D

// Device size parameters
#define AT24C02_PAGE_SIZE           32
#define AT24C02_PAGE_BYTE_SIZE      8

const eepromVTable_t at24c02_vTable;

static void at24c02_configure(eepromDevice_t *edevice, uint32_t configurationFlags)
{
    UNUSED(edevice);
    UNUSED(configurationFlags);
}

static bool at24c02_isReady(eepromDevice_t *edevice)
{
    UNUSED(edevice);

    return !i2cBusBusy(&hi2c1, NULL);
}

static void at24c02_program(eepromDevice_t *edevice, uint32_t address, const uint8_t *data, uint32_t length)
{
    UNUSED(edevice);

    uint8_t currentPage;
    uint8_t currentPageIndex;
    uint8_t pageDataCount;
    uint8_t offset;

    currentPage = address / AT24C02_PAGE_BYTE_SIZE;
    currentPageIndex = address % AT24C02_PAGE_BYTE_SIZE;

    if (length + currentPageIndex > AT24C02_PAGE_BYTE_SIZE) {
        pageDataCount = AT24C02_PAGE_BYTE_SIZE - currentPageIndex;
    } else {
        pageDataCount = length;
    }

    offset = 0;

    i2cBusWriteRegisterBufferStart(&hi2c1, AT24C02_I2C_ADDRESS, (currentPage * AT24C02_PAGE_BYTE_SIZE + currentPageIndex), (uint8_t *)&data[offset], pageDataCount);
    offset += pageDataCount;
    length -= pageDataCount;
    currentPage++;

    while (length > 0) {
        pageDataCount = length;
        if (pageDataCount > AT24C02_PAGE_BYTE_SIZE) {
            pageDataCount = AT24C02_PAGE_BYTE_SIZE;
        }

        i2cBusWriteRegisterBufferStart(&hi2c1, AT24C02_I2C_ADDRESS, (currentPage * AT24C02_PAGE_BYTE_SIZE), (uint8_t *)&data[offset], pageDataCount);
        offset += pageDataCount;
        length -= pageDataCount;
        currentPage++;
    }
}

static int at24c02_readBytes(eepromDevice_t *edevice, uint32_t address, uint8_t *buffer, uint32_t length)
{
    UNUSED(edevice);

    i2cBusReadRegisterBufferStart(&hi2c1, AT24C02_I2C_ADDRESS, address, buffer, length);

    return length;
}

static bool at24c02_identify(eepromDevice_t *edevice)
{
    bool isDetected = false;

    uint16_t data;

    while (!at24c02_isReady(edevice));

    at24c02_readBytes(edevice, 0x00, (uint8_t *)&data, 2);
    while (!at24c02_isReady(edevice));

    if (data == AT24C02_MAGICNUMBER) {
        edevice->vTable = &at24c02_vTable;
        isDetected = true;
    } else {
        data = AT24C02_MAGICNUMBER;
        at24c02_program(edevice, 0x00, (uint8_t *)&data, 2);
        while (!at24c02_isReady(edevice));

        data = 0x00;

        at24c02_readBytes(edevice, 0x00, (uint8_t *)&data, 2);
        while (!at24c02_isReady(edevice));

        if (data == AT24C02_MAGICNUMBER) {
            edevice->vTable = &at24c02_vTable;
            isDetected = true;
        } else {
            isDetected = false;
        }
    }

    return isDetected;
}

const eepromVTable_t at24c02_vTable = {
    .configure = at24c02_configure,
    .isReady = at24c02_isReady,
    .program = at24c02_program,
    .readBytes = at24c02_readBytes,
};

bool at24c02_init(eepromDevice_t *edevice)
{
    return at24c02_identify(edevice);
}