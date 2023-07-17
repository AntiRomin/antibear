#include <stdint.h>

#include "platform.h"

#include "ff.h"

#include "FatFs/diskio_quad_flash.h"

int QUADFLASH_disk_status(void)
{
    return 0;
}

int QUADFLASH_disk_initiallize(void)
{
    return 0;
}

int QUADFLASH_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
    UNUSED(buff);
    UNUSED(sector);
    UNUSED(count);
}

int QUADFLASH_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    UNUSED(buff);
    UNUSED(sector);
    UNUSED(count);
}

void QUADFLASH_disk_ioctl(BYTE cmd, void *buff)
{
    UNUSED(cmd);
    UNUSED(buff);
}
