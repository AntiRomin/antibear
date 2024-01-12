#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "flash.h"
#include "flash_impl.h"
#include "flash_w25q256jv.h"
#include "drivers/bus_quadspi.h"
#include "drivers/time.h"
#include "drivers/system.h"

static flashDevice_t flashDevice[FLASH_ID_COUNT];

bool flashDeviceDetect(void)
{
    bool detected = false;

    for (uint8_t index = 0; index < FLASH_ID_COUNT; index++) {
        switch (index) {
            case FLASH_ID_W25Q256JV:
            {
                detected |= w25q256jv_init(&flashDevice[index]);
                if (detected && flashDevice[index].vTable->configure) {
                    flashDevice[index].vTable->configure(&flashDevice[index], 0);
                }
            } break;
        }
    }

    return detected;
}

bool flashIsReady(flashId_e id)
{
    return flashDevice[id].vTable->isReady(&flashDevice[id]);
}

bool flashWaitForReady(flashId_e id)
{
    return flashDevice[id].vTable->waitForReady(&flashDevice[id]);
}

void flashEraseSector(flashId_e id, uint32_t address)
{
    flashDevice[id].callback = NULL;
    flashDevice[id].vTable->eraseSector(&flashDevice[id], address);
}

void flashEraseCompletely(flashId_e id)
{
    flashDevice[id].callback = NULL;
    flashDevice[id].vTable->eraseCompletely(&flashDevice[id]);
}

/**
 * The callback, if provided, will receive the totoal number of bytes transferd
 * by each call to flashPageProgramContinue() once the transfer completes.
*/
void flashPageProgramBegin(flashId_e id, uint32_t address, void (*callback)(uint32_t length))
{
    flashDevice[id].vTable->pageProgramBegin(&flashDevice[id], address, callback);
}

uint32_t flashPageProgramContinue(flashId_e id, const uint8_t **buffers, uint32_t *bufferSizes, uint32_t bufferCount)
{
    uint32_t maxBytesToWrite = flashDevice[id].geometry.pageSize - (flashDevice[id].currentWriteAddress % flashDevice[id].geometry.pageSize);

    if (bufferCount == 0) {
        return 0;
    }

    if (bufferSizes[0] >= maxBytesToWrite) {
        bufferSizes[0] = maxBytesToWrite;
        bufferCount = 1;
    } else {
        maxBytesToWrite -= bufferSizes[0];
        if ((bufferCount == 2) && (bufferSizes[1] > maxBytesToWrite)) {
            bufferSizes[1] = maxBytesToWrite;
        }
    }

    return flashDevice[id].vTable->pageProgramContinue(&flashDevice[id], buffers, bufferSizes, bufferCount);
}

void flashPageProgramFinish(flashId_e id)
{
    flashDevice[id].vTable->pageProgramFinish(&flashDevice[id]);
}

void flashPageProgram(flashId_e id, uint32_t address, const uint8_t *data, uint32_t length, void (*callback)(uint32_t length))
{
    flashDevice[id].vTable->pageProgram(&flashDevice[id], address, data, length, callback);
}

int flashReadBytes(flashId_e id, uint32_t address, uint8_t *buffer, uint32_t length)
{
    flashDevice[id].callback = NULL;
    return flashDevice[id].vTable->readBytes(&flashDevice[id], address, buffer, length);
}

void flashFlush(flashId_e id)
{
    if (flashDevice[id].vTable->flush) {
        flashDevice[id].vTable->flush(&flashDevice[id]);
    }
}

static const flashGeometry_t noFlashGeometry = {
    .totoalSize = 0,
};

const flashGeometry_t *flashGetGeometry(flashId_e id)
{
    if (flashDevice[id].vTable && flashDevice[id].vTable->getGeometry) {
        return flashDevice[id].vTable->getGeometry(&flashDevice[id]);
    }

    return &noFlashGeometry;
}

bool flashDeviceInit(void)
{
    bool haveFlash = false;

    haveFlash = flashDeviceDetect();

    return haveFlash;
}

bool flashInit(void)
{
    bool haveFlash = flashDeviceInit();

    return haveFlash;
}