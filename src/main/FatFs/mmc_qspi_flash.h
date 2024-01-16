#pragma once

DRESULT qspi_flash_initialize(void);
DRESULT qspi_flash_read(BYTE *buff, LBA_t sector, UINT count);
DRESULT qspi_flash_write(const BYTE *buff, LBA_t sector, UINT count);
DRESULT qspi_flash_ioctl(BYTE cmd, void *buff);