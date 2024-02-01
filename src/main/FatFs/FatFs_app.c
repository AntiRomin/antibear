#include "platform.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ff.h"
#include "FatFs_app.h"

typedef struct {
    FATFS FatFs;
    FILINFO Finfo;
    FIL File;
    DIR Dir;

    char *fs_type;      // FAT type
    DWORD bc;           // Bytes/Cluster
    BYTE n_fats;        // Number of FATs
    WORD n_rootdir;     // Root DIR entries
    DWORD fsize;        // Sectors/FAT
    DWORD n_clusters;   // Number of clusters
    LBA_t volbase;      // Volume start (lba)
    LBA_t fatbase;      // Fat start (lba)
    LBA_t dirbase;      // DIR start (lba)
    LBA_t database;     // Data start (lba)
#if FF_USE_LABEL
    DWORD vsn;          // Volume S/N
#endif
    UINT acc_dirs;      // folders
    UINT acc_files;     // files
    QWORD acc_size;     // bytes
    QWORD acc_total;    // KiB total disk space
    DWORD acc_use;      // KiB available
} FatFs_runtime_t;

FatFs_runtime_t FatFs_runtime;

char Line[256];
BYTE Buff[4096] __attribute__((aligned(4)));

static const char *ft[] = {"", "FAT12", "FAT16", "FAT32", "exFAT"};

/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */
/*--------------------------------------------------------------------------*/

static FRESULT scan_files (
    char* path,     /* Pointer to the path name working buffer */
    UINT* n_dir,
    UINT* n_file,
    QWORD* sz_file
)
{
    DIR dirs;
    FRESULT res;
    BYTE i;

    if ((res = f_opendir(&dirs, path)) == FR_OK) {
        i = strlen(path);
        while (((res = f_readdir(&dirs, &FatFs_runtime.Finfo)) == FR_OK) && FatFs_runtime.Finfo.fname[0]) {
            if (FatFs_runtime.Finfo.fattrib & AM_DIR) {
                (*n_dir)++;
                *(path+i) = '/'; strcpy(path+i+1, FatFs_runtime.Finfo.fname);
                res = scan_files(path, n_dir, n_file, sz_file);
                *(path+i) = '\0';
                if (res != FR_OK) break;
            } else {
                // xprintf("%s/%s\n", path, fn);
                (*n_file)++;
                *sz_file += FatFs_runtime.Finfo.fsize;
            }
        }
    }

    return res;
}

static void FatFs_appUpdateStatus(void)
{
    FATFS *fs;
    FRESULT res;
    DWORD value;

    res = f_getfree("0:", &value, &fs);
    if (res) return;

    FatFs_runtime.fs_type = (char *)ft[fs->fs_type];
    FatFs_runtime.bc = (DWORD)fs->csize * 512;
    FatFs_runtime.n_fats = fs->n_fats;
    if (fs->fs_type < FS_FAT32) FatFs_runtime.n_rootdir = fs->n_rootdir;
    FatFs_runtime.fsize = fs->fsize;
    FatFs_runtime.n_clusters = fs->n_fatent - 2;
    FatFs_runtime.volbase = fs->volbase;
    FatFs_runtime.fatbase = fs->fatbase;
    FatFs_runtime.dirbase = fs->dirbase;
    FatFs_runtime.database = fs->database;
#if FF_USE_LABEL
    res = f_getlabel("0:", (char *)Buff, (DWORD *)&FatFs_runtime.vsn);
    if (res) return;

#endif

    Line[0] = '0';
    Line[1] = ':';
    Line[2] = '\0';
    char *ptr = Line;
    res = scan_files(ptr, &FatFs_runtime.acc_dirs, &FatFs_runtime.acc_files, &FatFs_runtime.acc_size);
    if (res) return;

    FatFs_runtime.acc_total = (fs->n_fatent - 2) * (fs->csize / 2);
    FatFs_runtime.acc_use = (DWORD)value * (fs->csize / 2);
}

void FatFs_appInit(void)
{
    memset(&FatFs_runtime, 0x00, sizeof(FatFs_runtime_t));

    FRESULT res;

    res = f_mount(&FatFs_runtime.FatFs, "0:", 1);

    if (res == FR_NO_FILESYSTEM) {
        res = f_mkfs("0:", NULL, Buff, sizeof(Buff));

        res = f_mount(NULL, "0:", 1);
        res = f_mount(&FatFs_runtime.FatFs, "0:", 1);
    }

    FatFs_appUpdateStatus();

    res = f_open(&FatFs_runtime.File, "0:FatFs读写测试文件.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res == FR_OK) {
        BYTE WriteBuffer[] = "It's Romin here, and i am testing the writing function of the FatFs\r\n";
        UINT cnt;
        res = f_write(&FatFs_runtime.File, WriteBuffer, sizeof(WriteBuffer), &cnt);
        f_close(&FatFs_runtime.File);
    }

    res = f_open(&FatFs_runtime.File, "0:FatFs读写测试文件.txt", FA_OPEN_EXISTING | FA_READ);
    if (res == FR_OK) {
        UINT cnt;
        res = f_read(&FatFs_runtime.File, Buff, sizeof(Buff), &cnt);
    }
    f_close(&FatFs_runtime.File);
    f_mount(NULL, "0:", 1);
}