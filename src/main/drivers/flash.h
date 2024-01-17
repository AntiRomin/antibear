#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FLASH_TYPE_NOR = 0,
    FLASH_TYPE_NAND,
} flashType_e;

typedef uint16_t flashSector_t;

typedef struct flashGeometry_s {
    flashSector_t sectors; // Count of the number of erasable blocks on the device
    uint16_t pageSize; // In bytes
    uint32_t sectorSize; // This is just pagesPerSector * pageSize
    uint32_t totoalSize; // This is just sectorSize * sectors
    uint16_t pagesPerSector;
    flashType_e flashType;
    uint32_t jedecId;
} flashGeometry_t;

typedef enum {
    /*
     * When set it indicates the system was booted in memory mapped mode, flash chip is already configured by
     * the bootloader and does not need re-configuration.
     * When un-set the system was booted normally and the flash chip needs configuration before use.
     */
    FLASH_CF_SYSTEM_IS_MEMORY_MAPPED  = (1 << 0),
} flashConfigurationFlags_e;

typedef enum {
    FLASH_ID_W25Q256JV = 0,
    FLASH_ID_COUNT,
} flashId_e;

bool flashInit(void);

bool flashIsReady(flashId_e id);
bool flashWaitForReady(flashId_e id);
void flashEraseSector(flashId_e id, uint32_t address);
void flashEraseCompletely(flashId_e id);
void flashPageProgramBegin(flashId_e id, uint32_t address, void (*callback)(uint32_t length));
uint32_t flashPageProgramContinue(flashId_e id, const uint8_t **buffers, uint32_t *bufferSizes, uint32_t bufferCount);
void flashPageProgramFinish(flashId_e id);
void flashPageProgram(flashId_e id, uint32_t address, const uint8_t *data, uint32_t length, void (*callback)(uint32_t length));
int flashReadBytes(flashId_e id, uint32_t address, uint8_t *buffer, uint32_t length);
void flashFlush(flashId_e id);
const flashGeometry_t *flashGetGeometry(flashId_e id);