#include "platform.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ff.h"
#include "FatFs_app.h"

BYTE Buff[4096] __attribute__((aligned(4)));

FATFS FatFs;
FRESULT res;

void FatFs_appInit(void)
{
    res = f_mount(&FatFs, "0:", 1);

    if (res == FR_NO_FILESYSTEM) {
        res = f_mkfs("0:", NULL, Buff, sizeof(Buff));
    }
}