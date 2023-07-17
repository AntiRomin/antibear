#include <stdint.h>

#include "ff.h"

int QUADFLASH_disk_status(void);
int QUADFLASH_disk_initiallize(void);
int QUADFLASH_disk_read(BYTE *buff, LBA_t sector, UINT count);
int QUADFLASH_disk_write(const BYTE *buff, LBA_t sector, UINT count);
void QUADFLASH_disk_ioctl(BYTE cmd, void *buff);