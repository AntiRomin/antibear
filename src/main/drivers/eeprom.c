#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "eeprom.h"
#include "eeprom_impl.h"
#include "eeprom_at24c02.h"
#include "drivers/bus_i2c.h"

static eepromDevice_t eepromDevice[EEPROM_ID_COUNT];

bool eepromDeviceDetect(void)
{
    bool detected = false;

    for (uint8_t index = 0; index < EEPROM_ID_COUNT; index++) {
        switch (index) {
            case EEPROM_ID_AT24C02:
            {
                detected |= at24c02_init(&eepromDevice[index]);
                if (detected && eepromDevice[index].vTable->configure) {
                    eepromDevice[index].vTable->configure(&eepromDevice[index], 0);
                }
            } break;
        }
    }

    return detected;
}

bool eepromIsReady(eepromId_e id)
{
    return eepromDevice[id].vTable->isReady(&eepromDevice[id]);
}

void eepromProgram(eepromId_e id, uint32_t address, const uint8_t *buffer, uint32_t length)
{
    eepromDevice[id].vTable->program(&eepromDevice[id], address, buffer, length);
}

int eepromReadBytes(eepromId_e id, uint32_t address, uint8_t *buffer, uint32_t length)
{
    return eepromDevice[id].vTable->readBytes(&eepromDevice[id], address, buffer, length);
}

bool eepromDeviceInit(void)
{
    bool haveEeprom = false;

    haveEeprom = eepromDeviceDetect();

    return haveEeprom;
}

bool eepromInit(void)
{
    bool haveEeprom = eepromDeviceInit();

    return haveEeprom;
}