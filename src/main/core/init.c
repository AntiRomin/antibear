#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "platform.h"

#include "build/debug.h"

#include "core/init.h"

#include "drivers/bus_quadspi.h"
#include "drivers/flash.h"
#include "drivers/system.h"
#include "drivers/time.h"

uint8_t systemState = SYSTEM_STATE_INITIALISING;

uint8_t writeData[16384];
uint8_t readData[16384] = {0};

void init(void)
{
    systemInit();

    quadSpiInit();

    flashInit();

    debugInit();

    const flashGeometry_t *flashGeometry = flashGetGeometry(FLASH_ID_W25Q256JV);

    uint32_t flashSectorSize = flashGeometry->sectorSize;
    uint32_t flashPageSize = flashGeometry->pageSize;

    uint32_t flashStartAddress = 0;

    uint32_t bytesRemaining = 16384;
    uint32_t offset = 0;

    for (uint32_t i = 0; i < 16384; i++) {
        writeData[i] = 6;
    }

    do {
        uint32_t flashAddress = flashStartAddress + offset;

        uint32_t bytesToWrite = bytesRemaining;
        if (bytesToWrite > flashPageSize) {
            bytesToWrite = flashPageSize;
        }

        bool onSectorBoundary = flashAddress % flashSectorSize == 0;
        if (onSectorBoundary) {
            flashEraseSector(FLASH_ID_W25Q256JV, flashAddress);
        }

        flashPageProgram(FLASH_ID_W25Q256JV, flashAddress, (uint8_t *)&writeData[offset], bytesToWrite, NULL);

        bytesRemaining -= bytesToWrite;
        offset += bytesToWrite;
    } while (bytesRemaining > 0);

    uint32_t totalBytesRead = 0;
    int bytesRead = 0;

    bool success = false;

    do {
        bytesRead = flashReadBytes(FLASH_ID_W25Q256JV, flashStartAddress + totalBytesRead, &readData[totalBytesRead], 16384 - totalBytesRead);
        if (bytesRead > 0) {
            totalBytesRead += bytesRead;
            success = (totalBytesRead == 16384);
        }
    } while (!success && bytesRead > 0);

    flashWaitForReady(FLASH_ID_W25Q256JV);

    systemState |= SYSTEM_STATE_READY;
}