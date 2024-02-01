/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"         /* Obtains integer types */
#include "diskio.h"     /* Declarations of disk functions */

#include "common/time.h"
#include "mmc_qspi_flash.h"

/* Definitions of physical drive number for each drive */
typedef enum
{
    DEV_QSPI_FLASH = 0x00,
    DEV_COUNT,
} dev_e;

static volatile DSTATUS Stat[DEV_COUNT] = {STA_NOINIT};  /* Physical drive status*/

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv       /* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv) {
        case DEV_QSPI_FLASH:
        {
            return Stat[pdrv];
        } break;

        default:
        {
            return STA_NOINIT;
        } break;
    }
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv               /* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv) {
        case DEV_QSPI_FLASH:
        {
            if (qspi_flash_initialize() == RES_OK) {
                Stat[pdrv] &= ~STA_NOINIT;
            } else {
                Stat[pdrv] = STA_NOINIT;
            }
        } break;

        default:
        {
            return STA_NOINIT;
        } break;
    }

    return Stat[pdrv];
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE *buff,     /* Data buffer to store read data */
    LBA_t sector,   /* Start sector in LBA */
    UINT count      /* Number of sectors to read */
)
{
    if (!count) {   /* Check parameter */
        return RES_PARERR;
    }

    switch (pdrv) {
        case DEV_QSPI_FLASH:
        {
            if (Stat[pdrv] & STA_NOINIT) {   /* Check if drive is ready */
                return RES_NOTRDY;
            }

            return qspi_flash_read(buff, sector, count);
        } break;

        default:
        {
            return RES_PARERR;
        } break;
    }
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE *buff,   /* Data to be written */
    LBA_t sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    if (!count) {   /* Check parameter */
        return RES_PARERR;
    }

    switch (pdrv) {
        case DEV_QSPI_FLASH:
        {
            if (Stat[pdrv] & STA_NOINIT) {   /* Check if drive is ready */
                return RES_NOTRDY;
            }

            return qspi_flash_write(buff, sector, count);
        }

        default:
        {
            return RES_PARERR;
        } break;
    }
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    switch (pdrv) {
        case DEV_QSPI_FLASH:
        {
            return qspi_flash_ioctl(cmd, buff);
        }

        default:
        {
            return RES_PARERR;
        } break;
    }
}

/*---------------------------------------------------------*/
/* User provided RTC function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called back     */
/* from FatFs module.                                      */

#if !FF_FS_NORTC && !FF_FS_READONLY
DWORD get_fattime (void)
{
    dateTime_t utcDateTime;
    dateTime_t localDateTime;

    /* Get local time */
    if (!rtcGetDateTime(&utcDateTime)) {
        localDateTime.year = RTC_DEFAULT_YEAR + 2000;
        localDateTime.month = RTC_DEFAULT_DATE;
        localDateTime.date = RTC_DEFAULT_MONTH;
        localDateTime.hours = RTC_DEFAULT_HOUR;
        localDateTime.minutes = RTC_DEFAULT_MINUTE;
        localDateTime.seconds = RTC_DEFAULT_SECOND;
    } else {
        dateTimeUTCToLocal(&utcDateTime, &localDateTime);
    }

    /* Pack date and time into a DWORD variable */
    return    ((DWORD)(localDateTime.year - 1980) << 25)
            | ((DWORD)localDateTime.month << 21)
            | ((DWORD)localDateTime.date << 16)
            | ((DWORD)localDateTime.hours << 11)
            | ((DWORD)localDateTime.minutes << 5)
            | ((DWORD)localDateTime.seconds >> 1);
}
#endif