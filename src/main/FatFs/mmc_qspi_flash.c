#include <string.h>
#include "platform.h"

#include "ff.h"
#include "diskio.h"

#include "mmc_qspi_flash.h"
#include "drivers/flash.h"

DRESULT qspi_flash_initialize(void)
{
    // qspi flash already initialized at inti()
    return RES_OK;
}

DRESULT qspi_flash_read(BYTE *buff, LBA_t sector, UINT count)
{
    LBA_t baseAddress;
    DWORD lenght;

    const flashGeometry_t *flashGeometry = flashGetGeometry(FLASH_ID_W25Q256JV);
    DWORD flashSectorSize = flashGeometry->sectorSize;

#if defined(USE_QSPI_DUALFLASH)
    UINT isShift;

    baseAddress = (sector / 2) * flashSectorSize;
    isShift = sector % 2;
    if (isShift) {
        baseAddress += flashSectorSize / 2;
    }

    lenght = count * (flashSectorSize / 2);
#else
    baseAddress = sector * flashSectorSize;
    lenght = count * flashSectorSize;
#endif

    flashReadBytes(FLASH_ID_W25Q256JV, baseAddress, buff, lenght);
    return RES_OK;
}

DRESULT qspi_flash_write(const BYTE *buff, LBA_t sector, UINT count)
{
    LBA_t baseAddress;
    LBA_t flashAddress;
    DWORD lenght;
    DWORD buffIndexOffset;
    DWORD offset;
    DWORD bytesToWrite;

    const flashGeometry_t *flashGeometry = flashGetGeometry(FLASH_ID_W25Q256JV);
    DWORD flashSectorSize = flashGeometry->sectorSize;
    DWORD flashPageSize = flashGeometry->pageSize;

    buffIndexOffset = 0;

#if defined(USE_QSPI_DUALFLASH)
    UINT isShift;
    LBA_t lastAddress;
    unsigned char tmpbuff[8192];

    baseAddress = (sector / 2) * flashSectorSize;

    isShift = sector % 2;
    if (isShift) {
        flashReadBytes(FLASH_ID_W25Q256JV, baseAddress, tmpbuff, flashSectorSize);
        memcpy(&tmpbuff[4096], &buff[0], (flashSectorSize / 2));

        flashEraseSector(FLASH_ID_W25Q256JV, baseAddress);

        offset = 0;
        lenght = 8192;
        do {
            flashAddress = baseAddress + offset;

            bytesToWrite = lenght;
            if (bytesToWrite > flashPageSize) {
                bytesToWrite = flashPageSize;
            }

            flashPageProgram(FLASH_ID_W25Q256JV, flashAddress, &tmpbuff[offset], bytesToWrite, NULL);

            lenght -= bytesToWrite;
            offset += bytesToWrite;
        } while (lenght > 0);

        baseAddress += (flashSectorSize / 2);
        buffIndexOffset = (flashSectorSize / 2);
        count--;
    }

    if (!count)
        return RES_OK;

    isShift = (sector + count) % 2; 
    if (isShift) {
        lastAddress = ((sector + count) / 2) * flashSectorSize;

        flashReadBytes(FLASH_ID_W25Q256JV, lastAddress, tmpbuff, flashSectorSize);
        memcpy(&tmpbuff[0], &buff[lastAddress - (baseAddress + buffIndexOffset)], (flashSectorSize / 2));

        flashEraseSector(FLASH_ID_W25Q256JV, lastAddress);

        offset = 0;
        lenght = 8192;
        do {
            flashAddress = lastAddress + offset;

            bytesToWrite = lenght;
            if (bytesToWrite > flashPageSize) {
                bytesToWrite = flashPageSize;
            }

            flashPageProgram(FLASH_ID_W25Q256JV, flashAddress, &tmpbuff[offset], bytesToWrite, NULL);

            lenght -= bytesToWrite;
            offset += bytesToWrite;
        } while (lenght > 0);

        count--;
    }

    if (!count)
        return RES_OK;

    lenght = count * (flashSectorSize / 2);

    for (UINT index = 0; index < (count / 2); index++) {
        flashEraseSector(FLASH_ID_W25Q256JV, (sector + index) * flashSectorSize);
    }
#else
    baseAddress = sector * flashSectorSize;
    lenght = count * flashSectorSize;

    for (UINT index = 0; index < count; index++) {
        flashEraseSector(FLASH_ID_W25Q256JV, (sector + index) * flashSectorSize);
    }
#endif

    offset = 0;
    do {
        flashAddress = baseAddress + offset;

        bytesToWrite = lenght;
        if (bytesToWrite > flashPageSize) {
            bytesToWrite = flashPageSize;
        }

        flashPageProgram(FLASH_ID_W25Q256JV, flashAddress, &buff[buffIndexOffset + offset], bytesToWrite, NULL);

        lenght -= bytesToWrite;
        offset += bytesToWrite;
    } while (lenght > 0);

    return RES_OK;
}

DRESULT qspi_flash_ioctl(BYTE cmd, void *buff)
{
    DRESULT res = RES_ERROR;

    const flashGeometry_t *flashGeometry = flashGetGeometry(FLASH_ID_W25Q256JV);
    DWORD flashSectors = flashGeometry->sectors;
    DWORD flashSectorSize = flashGeometry->sectorSize;

    switch (cmd) {
        case CTRL_SYNC:
        {
            if (flashIsReady(FLASH_ID_W25Q256JV)) {
                res = RES_OK;
            }
        } break;

        case GET_SECTOR_COUNT:
        {
            *(LBA_t*)buff = flashSectors;
            res = RES_OK;
        } break;

        case GET_SECTOR_SIZE:
        {
#if defined(USE_QSPI_DUALFLASH)
            *(LBA_t*)buff = flashSectorSize / 2;
#else
            *(LBA_t*)buff = flashSectorSize;
#endif
            res = RES_OK;
        } break;

        case GET_BLOCK_SIZE:
        {
            *(DWORD*)buff = 1;
            res = RES_OK;
        } break;

#if defined(FF_USE_TRIM)
        case CTRL_TRIM:
        {
            res = RES_OK;
        } break;
#endif

        default:
        {
            res = RES_PARERR;
        } break;
    }

    return res;
}