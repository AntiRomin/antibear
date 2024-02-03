#pragma once

struct eepromVTable_s;

typedef struct eepromDevice_s {
    //
    // members to be configured by the eeprom chip implementation
    //

    const struct eepromVTable_s *vTable;
} eepromDevice_t;

typedef struct eepromVTable_s {
    void (*configure)(eepromDevice_t *edevice, uint32_t configurationFlags);

    bool (*isReady)(eepromDevice_t *edevice);

    void (*program)(eepromDevice_t *edevice, uint32_t address, const uint8_t *data, uint32_t length);

    int (*readBytes)(eepromDevice_t *edevice, uint32_t address, uint8_t *buffer, uint32_t length);
} eepromVTable_t;