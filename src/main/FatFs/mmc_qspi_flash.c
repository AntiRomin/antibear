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
    LBA_t lenght;

#if defined(USE_QSPI_DUALFLASH)
    UINT isShift;

    baseAddress = (sector >> 1) * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize;
    isShift = sector % 2;
    if (isShift) {
        baseAddress += flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize >> 1;
    }

    lenght = count * (flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize >> 1);
#else
    baseAddress = sector * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize;
    lenght = count * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize;
#endif

    flashReadBytes(FLASH_ID_W25Q256JV, baseAddress, buff, lenght);
    return RES_OK;
}

DRESULT qspi_flash_write(const BYTE *buff, LBA_t sector, UINT count)
{
    LBA_t baseAddress;
    LBA_t lenght;

#if defined(USE_QSPI_DUALFLASH)
    UINT isShift;
    LBA_t buffIndexOffset;
    LBA_t lastAddress;
    unsigned char tmpbuff[8192];

    buffIndexOffset = 0;
    baseAddress = (sector >> 1) * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize;

    isShift = sector % 2;
    if (isShift) {
        flashReadBytes(FLASH_ID_W25Q256JV, baseAddress, tmpbuff, flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize);
        memcpy(&tmpbuff[4096], &buff[0], (flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize >> 1));

        flashEraseSector(FLASH_ID_W25Q256JV, baseAddress);
        flashPageProgram(FLASH_ID_W25Q256JV, baseAddress, tmpbuff, flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize, NULL);

        baseAddress += flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize >> 1;
        buffIndexOffset = (flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize >> 1);
        count--;
    }

    if (!count)
        return RES_OK;

    isShift = (sector + count) % 2; 
    if (isShift) {
        lastAddress = ((sector + count) >> 1) * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize;

        flashReadBytes(FLASH_ID_W25Q256JV, lastAddress, tmpbuff, flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize);
        memcpy(&tmpbuff[0], &buff[lastAddress], (flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize >> 1));

        flashEraseSector(FLASH_ID_W25Q256JV, lastAddress);
        flashPageProgram(FLASH_ID_W25Q256JV, lastAddress, tmpbuff, flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize, NULL);

        count--;
    }

    if (!count)
        return RES_OK;

    lenght = count * (flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize >> 1);

    for (UINT index = 0; index < (count >> 1); index++) {
        flashEraseSector(FLASH_ID_W25Q256JV, (sector + index) * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize);
    }
#else
    baseAddress = sector * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize;
    lenght = count * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize;

    for (UINT index = 0; index < count; index++) {
        flashEraseSector(FLASH_ID_W25Q256JV, (sector + index) * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize);
    }
#endif

    flashPageProgram(FLASH_ID_W25Q256JV, baseAddress, &buff[buffIndexOffset], lenght, NULL);
    return RES_OK;
}

DRESULT qspi_flash_ioctl(BYTE cmd, void *buff)
{
    DRESULT res = RES_ERROR;

    switch (cmd) {
        case CTRL_SYNC:
        {
            if (flashIsReady(FLASH_ID_W25Q256JV)) {
                res = RES_OK;
            }
        } break;

        case GET_SECTOR_COUNT:
        {
            *(LBA_t*)buff = flashGetGeometry(FLASH_ID_W25Q256JV)->sectors;
            res = RES_OK;
        } break;

        case GET_SECTOR_SIZE:
        {
#if defined(USE_QSPI_DUALFLASH)
            *(LBA_t*)buff = flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize >> 1;
#else
            *(LBA_t*)buff = flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize;
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