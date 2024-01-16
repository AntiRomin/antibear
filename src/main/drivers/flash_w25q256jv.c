#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/flash.h"
#include "drivers/flash_impl.h"
#include "drivers/flash_w25q256jv.h"
#include "drivers/bus_quadspi.h"

#define USE_FLASH_WRITES_USING_4LINES
#define USE_FLASH_READS_USING_4LINES

// JEDEC ID
#define JEDEC_ID_WINBOND_W25Q256JV              0xEF4019

// Device size parameters
#define W25Q256JV_PAGE_SIZE         2048
#define W25Q256JV_PAGE_BYTE_SIZE    256
#define W25Q256JV_PAGES_PER_BLOCK   256
#define W25Q256JV_BLOCKS_PER_DIE    512
#define W25Q256JV_BLOCK_SIZE        (W25Q256JV_PAGES_PER_BLOCK * W25Q256JV_PAGE_SIZE)

// Sizes
#if !defined(USE_QSPI_DUALFLASH)
#define W25Q256JV_STATUS_REGISTER_BITS        8
#define W25Q256JV_JEDEC_ID_LENGTH             24
#else
#define W25Q256JV_STATUS_REGISTER_BITS        16
#define W25Q256JV_JEDEC_ID_LENGTH             48
#endif

#define W25Q256JV_ADDRESS_BITS                32

// Instructions
#define W25Q256JV_INSTRUCTION_READ_DID              0xAB
#define W25Q256JV_INSTRUCTION_READ_MID_DID          0x90
#define W25Q256JV_INSTRUCTION_READ_MID_DID_DUAL_INOUT 0x92
#define W25Q256JV_INSTRUCTION_READ_MID_DID_QUAD_INOUT 0x94
#define W25Q256JV_INSTRUCTION_READ_UID              0x4B
#define W25Q256JV_INSTRUCTION_READ_JEDEC_ID         0x9F

#define W25Q256JV_INSTRUCTION_ENABLE_RESET          0x66
#define W25Q256JV_INSTRUCTION_RESET_DEVICE          0x99

#define W25Q256JV_INSTRUCTION_READ_STATUS1_REG      0x05
#define W25Q256JV_INSTRUCTION_READ_STATUS2_REG      0x35
#define W25Q256JV_INSTRUCTION_READ_STATUS3_REG      0x15

#define W25Q256JV_INSTRUCTION_WRITE_STATUS1_REG     0x01
#define W25Q256JV_INSTRUCTION_WRITE_STATUS2_REG     0x31
#define W25Q256JV_INSTRUCTION_WRITE_STATUS3_REG     0x11

#define W25Q256JV_INSTRUCTION_WRITE_ENABLE          0x06
#define W25Q256JV_INSTRUCTION_VOLATILE_WRITE_ENABLE 0x50
#define W25Q256JV_INSTRUCTION_WRITE_DISABLE         0x04
#define W25Q256JV_INSTRUCTION_SECTOR_ERASE          0x20
#define W25Q256JV_INSTRUCTION_SECTOR_ERASE_4B_ADDR  0x21
#define W25Q256JV_INSTRUCTION_BLOCK_ERASE_32KB      0x52
#define W25Q256JV_INSTRUCTION_BLOCK_ERASE_64KB      0xD8
#define W25Q256JV_INSTRUCTION_BLOCK_ERASE_64KB_4B_ADDR 0xDC
#define W25Q256JV_INSTRUCTION_CHIP_ERASE            0xC7

#define W25Q256JV_INSTRUCTION_4B_ADDR_ENABLE        0xB7
#define W25Q256JV_INSTRUCTION_4B_ADDR_DISABLE       0xE9

#define W25Q256JV_INSTRUCTION_READ_EX_ADDR_REG      0xC8
#define W25Q256JV_INSTRUCTION_WRITE_EX_ADDR_REG     0xC5

#define W25Q256JV_INSTRUCTION_READ                  0x03
#define W25Q256JV_INSTRUCTION_READ_4B_ADDR          0x13
#define W25Q256JV_INSTRUCTION_FAST_READ             0x0B
#define W25Q256JV_INSTRUCTION_FAST_READ_4B_ADDR     0x0C
#define W25Q256JV_INSTRUCTION_FAST_READ_DUAL_OUT    0x3B
#define W25Q256JV_INSTRUCTION_FAST_READ_DUAL_OUT_4B_ADDR 0x3C
#define W25Q256JV_INSTRUCTION_FAST_READ_QUAD_OUT    0x6B
#define W25Q256JV_INSTRUCTION_FAST_READ_QUAD_OUT_4B_ADDR 0x6C
#define W25Q256JV_INSTRUCTION_FAST_READ_DUAL_INOUT  0xBB
#define W25Q256JV_INSTRUCTION_FAST_READ_DUAL_INOUT_4B_ADDR 0xBC
#define W25Q256JV_INSTRUCTION_FAST_READ_QUAD_INOUT  0xEB
#define W25Q256JV_INSTRUCTION_FAST_READ_QUAD_INOUT_4B_ADDR 0xEC

#define W25Q256JV_INSTRUCTION_SET_BURST             0x77

#define W25Q256JV_INSTRUCTION_PAGE_PROGRAM          0x02
#define W25Q256JV_INSTRUCTION_PAGE_PROGRAM_4B_ADDR  0x12
#define W25Q256JV_INSTRUCTION_QUAD_PAGE_PROGRAM     0x32
#define W25Q256JV_INSTRUCTION_QUAD_PAGE_PROGRAM_4B_ADDR 0x34

#define W25Q256JV_INSTRUCTION_SUSPEND               0x75
#define W25Q256JV_INSTRUCTION_RESUMEN               0x7A

#define W25Q256JV_INSTRUCTION_POWER_DOWN            0xB9
#define W25Q256JV_INSTRUCTION_RELEASE_POWER_DOWN    0xAB

#define W25Q256JV_INSTRUCTION_READ_SFDP_REG         0x5A
#define W25Q256JV_INSTRUCTION_ERASE_SECURITY_REG    0x44
#define W25Q256JV_INSTRUCTION_PROGREAM_SECURITY_REG 0x42
#define W25Q256JV_INSTRUCTION_READ_SECURITY_REG     0x48

#define W25Q256JV_SR1_BIT_WRITE_IN_PROGRESS     (1 << 0)
#define W25Q256JV_SR1_BIT_WRITE_ENABLED         (1 << 1)
#define W25Q256JV_SR1_BITS_BLOCK_PROTECT        (0x0F << 2)

#define W25Q256JV_SR2_BIT_QUAD_ENABLE           (1 << 1)

#define W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE     (1 << 1)

// Values from W25Q256JV Datasheet.
#define W25Q256JV_TIMEOUT_PAGE_READ_MS          1           // No minimum specified in datasheet
#define W25Q256JV_TIMEOUT_RESET_MS              1           // tRST = 30us
#define W25Q256JV_TIMEOUT_SECTOR_ERASE_MS       400         // tBE2max = 400ms, tBE2typ = 50ms
#define W25Q256JV_TIMEOUT_BLOCK_ERASE_64KB_MS   2000        // tBE2max = 2000ms, tBE2typ = 150ms
#define W25Q256JV_TIMEOUT_CHIP_ERASE_MS        (400 * 1000) // tCEmax 400s, tCEtyp = 80s

#define W25Q256JV_TIMEOUT_PAGE_PROGRAM_MS       3           // tPPmax = 3ms, tPPtyp = 0.7ms
#define W25Q256JV_TIMEOUT_STATUS_REG_WRITE_MS   15          // tPPmax = 15ms, tPPtyp = 10ms
#define W25Q256JV_TIMEOUT_WRITE_ENABLE_MS       1

typedef struct w25q256jvState_s {
    uint32_t currentWriteAddress;
} w25q256jvState_t;

w25q256jvState_t w25q256jvState = { 0 };

static bool w25q256jv_waitForReady(flashDevice_t *fdevice);
static void w25q256jv_waitForTimeout(flashDevice_t *fdevice);

static void w25q256jv_setTimeout(flashDevice_t *fdevice, timeMs_t timeoutMillis)
{
    timeMs_t nowMs = microsISR() / 1000;
    fdevice->timeoutAt = nowMs + timeoutMillis;
}

static void w25q256jv_performOneByteCommand(uint8_t command)
{
    quadSpiTransmit1LINE(command, 0, NULL, 0);
}

static void w25q256jv_performCommandWithAddress(uint8_t command, uint32_t address)
{
    quadSpiInstructionWithAddress1LINE(command, 0, address, W25Q256JV_ADDRESS_BITS);
}

static void w25q256jv_writeEnable(flashDevice_t *fdevice)
{
    UNUSED(fdevice);
    w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_WRITE_ENABLE);
}

static uint16_t w25q256jv_readRegister(uint8_t command)
{
    uint8_t in[W25Q256JV_STATUS_REGISTER_BITS / 8] = { 0 };

    quadSpiReceive1LINE(command, 0, in, W25Q256JV_STATUS_REGISTER_BITS / 8);

#if !defined(USE_QSPI_DUALFLASH)
    return  in[0];
#else
    return (in[1] << 8 | in[0]);
#endif
}

static void w25q256jv_writeRegister(uint8_t command, uint16_t data)
{
    quadSpiTransmit1LINE(command, 0, (uint8_t *)&data, W25Q256JV_STATUS_REGISTER_BITS / 8);
}

static void w25q256jv_deviceReset(flashDevice_t *fdevice)
{
    w25q256jv_waitForReady(fdevice);
    w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_ENABLE_RESET);
    w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_RESET_DEVICE);

    w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_RESET_MS);
    w25q256jv_waitForTimeout(fdevice);

    w25q256jv_waitForReady(fdevice);

    uint16_t registerValue;

    registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS1_REG);

#if !defined(USE_QSPI_DUALFLASH)
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR1_BITS_BLOCK_PROTECT) != 0)
#else
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR1_BITS_BLOCK_PROTECT) != 0 ||
        (((uint8_t *)&registerValue)[1] & W25Q256JV_SR1_BITS_BLOCK_PROTECT) != 0)
#endif
    {
        // Disable block protect.
        registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS1_REG);

        uint16_t newValue = registerValue;
#if !defined(USE_QSPI_DUALFLASH)
        ((uint8_t *)&newValue)[0] &= ~W25Q256JV_SR1_BITS_BLOCK_PROTECT;
#else
        ((uint8_t *)&newValue)[0] &= ~W25Q256JV_SR1_BITS_BLOCK_PROTECT;
        ((uint8_t *)&newValue)[1] &= ~W25Q256JV_SR1_BITS_BLOCK_PROTECT;
#endif

        // w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_WRITE_ENABLE);
        w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_VOLATILE_WRITE_ENABLE);
        w25q256jv_writeRegister(W25Q256JV_INSTRUCTION_WRITE_STATUS1_REG, newValue);

        w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_STATUS_REG_WRITE_MS);
        w25q256jv_waitForTimeout(fdevice);
    }

    registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS2_REG);

#if !defined(USE_QSPI_DUALFLASH)
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR2_BIT_QUAD_ENABLE) == 0)
#else
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR2_BIT_QUAD_ENABLE) == 0 ||
        (((uint8_t *)&registerValue)[1] & W25Q256JV_SR2_BIT_QUAD_ENABLE) == 0)
#endif
    {
        // Enable QUADSPI mode.
        registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS2_REG);

        uint16_t newValue = registerValue;
#if !defined(USE_QSPI_DUALFLASH)
        ((uint8_t *)&newValue)[0] |= W25Q256JV_SR2_BIT_QUAD_ENABLE;
#else
        ((uint8_t *)&newValue)[0] |= W25Q256JV_SR2_BIT_QUAD_ENABLE;
        ((uint8_t *)&newValue)[1] |= W25Q256JV_SR2_BIT_QUAD_ENABLE;
#endif

        // w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_WRITE_ENABLE);
        w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_VOLATILE_WRITE_ENABLE);
        w25q256jv_writeRegister(W25Q256JV_INSTRUCTION_WRITE_STATUS2_REG, newValue);

        w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_STATUS_REG_WRITE_MS);
        w25q256jv_waitForTimeout(fdevice);
    }

    registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS3_REG);

#if !defined(USE_QSPI_DUALFLASH)
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE) == 0 )
#else
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE) == 0 ||
        (((uint8_t *)&registerValue)[1] & W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE) == 0)
#endif
    {
        // Enable 4 bytes address mode.
        registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS3_REG);

        uint16_t newValue = registerValue;
#if !defined(USE_QSPI_DUALFLASH)
        ((uint8_t *)&newValue)[0] |= W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE;
#else
        ((uint8_t *)&newValue)[0] |= W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE;
        ((uint8_t *)&newValue)[1] |= W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE;
#endif

        // w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_WRITE_ENABLE);
        w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_VOLATILE_WRITE_ENABLE);
        w25q256jv_writeRegister(W25Q256JV_INSTRUCTION_WRITE_STATUS3_REG, newValue);

        w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_STATUS_REG_WRITE_MS);
        w25q256jv_waitForTimeout(fdevice);
    }
}

bool w25q256jv_isReady(flashDevice_t *fdevice)
{
    UNUSED(fdevice);
    uint16_t status = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS1_REG);

#if !defined(USE_QSPI_DUALFLASH)
    bool busy = (((uint8_t *)&status)[0] & W25Q256JV_SR1_BIT_WRITE_IN_PROGRESS);
#else
    bool busy = ((((uint8_t *)&status)[0] & W25Q256JV_SR1_BIT_WRITE_IN_PROGRESS) ||
                 (((uint8_t *)&status)[1] & (W25Q256JV_SR1_BIT_WRITE_IN_PROGRESS)));
#endif

    return !busy;
}

static bool w25q256jv_isWritable(flashDevice_t *fdevice)
{
    UNUSED(fdevice);
    uint16_t status = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS1_REG);

#if !defined(USE_QSPI_DUALFLASH)
    bool writable = (((uint8_t *)&status)[0] & W25Q256JV_SR1_BIT_WRITE_ENABLED);
#else
    bool writable = ((((uint8_t *)&status)[0] & W25Q256JV_SR1_BIT_WRITE_ENABLED) &&
                     (((uint8_t *)&status)[1] & (W25Q256JV_SR1_BIT_WRITE_ENABLED)));
#endif

    return writable;
}

static bool w25q256jv_hasTimedOut(flashDevice_t *fdevice)
{
    uint32_t nowMs = microsISR() / 1000;
    if (cmp32(nowMs, fdevice->timeoutAt) >= 0) {
        return true;
    }
    return false;
}

static void w25q256jv_waitForTimeout(flashDevice_t *fdevice)
{
   while (!w25q256jv_hasTimedOut(fdevice)) { }

   fdevice->timeoutAt = 0;
}

static bool w25q256jv_waitForReady(flashDevice_t *fdevice)
{
    bool ready = true;
    while (!w25q256jv_isReady(fdevice)) {
        if (w25q256jv_hasTimedOut(fdevice)) {
            ready = false;
            break;
        }
    }
    fdevice->timeoutAt = 0;

    return ready;
}

const flashVTable_t w25q256jv_vTable;

static void w25q256jv_deviceInit(flashDevice_t *fdevice);

static bool w25q256jv_identify(flashDevice_t *fdevice)
{
    bool isDetected = false;

    uint8_t in[W25Q256JV_JEDEC_ID_LENGTH / 8];

    quadSpiReceive1LINE(W25Q256JV_INSTRUCTION_READ_JEDEC_ID, 0, in, W25Q256JV_JEDEC_ID_LENGTH / 8);

#if !defined(USE_QSPI_DUALFLASH)
    if ((in[0] << 16 | in[1] << 8 | in[2]) == JEDEC_ID_WINBOND_W25Q256JV)
#else
    if (((in[0] << 16 | in[2] << 8 | in[4]) == JEDEC_ID_WINBOND_W25Q256JV) &&
        ((in[1] << 16 | in[3] << 8 | in[5]) == JEDEC_ID_WINBOND_W25Q256JV))
#endif
    {
        fdevice->geometry.jedecId           = JEDEC_ID_WINBOND_W25Q256JV;

#if !defined(USE_QSPI_DUALFLASH)
        fdevice->geometry.sectors           = 8192;
        fdevice->geometry.pagesPerSector    = 16;
        fdevice->geometry.pageSize          = W25Q256JV_PAGE_BYTE_SIZE;
        // = 268435456 256MBit 32MB
#else
        fdevice->geometry.sectors           = 8192;
        fdevice->geometry.pagesPerSector    = 16;
        fdevice->geometry.pageSize          = W25Q256JV_PAGE_BYTE_SIZE * 2;
        // = (268435456 * 2) (256MBit * 2) 64MB
#endif

        fdevice->geometry.flashType         = FLASH_TYPE_NOR;
        fdevice->geometry.sectorSize        = fdevice->geometry.pagesPerSector * fdevice->geometry.pageSize;
        fdevice->geometry.totoalSize        = fdevice->geometry.sectorSize * fdevice->geometry.sectors;

        fdevice->vTable                     = &w25q256jv_vTable;

        isDetected = true;
    }

    return isDetected;
}

static void w25q256jv_configure(flashDevice_t *fdevice, uint32_t configurationFlags)
{
    if (configurationFlags & FLASH_CF_SYSTEM_IS_MEMORY_MAPPED) {
        return;
    }

    w25q256jv_deviceReset(fdevice);

    w25q256jv_deviceInit(fdevice);
}

static void w25q256jv_eraseSector(flashDevice_t *fdevice, uint32_t address)
{
    w25q256jv_waitForReady(fdevice);

    w25q256jv_writeEnable(fdevice);

    // verify write enable is set
    w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_WRITE_ENABLE_MS);
    bool writable = false;
    do {
        writable = w25q256jv_isWritable(fdevice);
    } while (!writable && w25q256jv_hasTimedOut(fdevice));

    if (!writable) {
        return;
    }

    w25q256jv_waitForReady(fdevice);

    w25q256jv_performCommandWithAddress(W25Q256JV_INSTRUCTION_SECTOR_ERASE_4B_ADDR, address);
    
    w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_SECTOR_ERASE_MS);
}

void w25q256jv_eraseCompletely(flashDevice_t *fdevice)
{
    w25q256jv_waitForReady(fdevice);

    w25q256jv_writeEnable(fdevice);

    // verify write enable is set
    w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_WRITE_ENABLE_MS);
    bool writable = false;
    do {
        writable = w25q256jv_isWritable(fdevice);
    } while (!writable && w25q256jv_hasTimedOut(fdevice));

    if (!writable) {
        return;
    }

    w25q256jv_waitForReady(fdevice);

    w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_CHIP_ERASE);

    w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_CHIP_ERASE_MS);
}

static void w25q256jv_loadProgramData(flashDevice_t *fdevice, const uint8_t *data, int length)
{
    w25q256jv_waitForReady(fdevice);

    quadSpiTransmitWithAddress4LINES(W25Q256JV_INSTRUCTION_QUAD_PAGE_PROGRAM_4B_ADDR, 0, w25q256jvState.currentWriteAddress, W25Q256JV_ADDRESS_BITS, data, length);

    w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_PAGE_PROGRAM_MS);

    w25q256jvState.currentWriteAddress += length;
}

static void w25q256jv_pageProgramBegin(flashDevice_t *fdevice, uint32_t address, void (*callback)(uint32_t length))
{
    fdevice->callback = callback;
    w25q256jvState.currentWriteAddress = address;
}

static uint32_t w25q256jv_pageProgramContinue(flashDevice_t *fdevice, uint8_t const **buffers, uint32_t *bufferSizes, uint32_t bufferCount)
{
    for (uint32_t i = 0; i < bufferCount; i++) {
        w25q256jv_waitForReady(fdevice);

        w25q256jv_writeEnable(fdevice);

        // verify write enable is set
        w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_WRITE_ENABLE_MS);
        bool writable = false;
        do {
            writable = w25q256jv_isWritable(fdevice);
        } while (!writable && w25q256jv_hasTimedOut(fdevice));

        if (!writable) {
            return 0;
        }

        w25q256jv_loadProgramData(fdevice, buffers[i], bufferSizes[i]);
    }

    return fdevice->callbackArg;
}

static void w25q256jv_pageProgramFinish(flashDevice_t *fdevice)
{
    UNUSED(fdevice);
}

static void w25q256jv_pageProgram(flashDevice_t *fdevice, uint32_t address, const uint8_t *data, uint32_t length, void (*callback)(uint32_t length))
{
    w25q256jv_pageProgramBegin(fdevice, address, callback);
    w25q256jv_pageProgramContinue(fdevice, &data, &length, 1);
    w25q256jv_pageProgramFinish(fdevice);
}

static void w25q256jv_flush(flashDevice_t *fdevice)
{
    UNUSED(fdevice);
}

static int w25q256jv_readBytes(flashDevice_t *fdevice, uint32_t address, uint8_t *buffer, uint32_t length)
{
    if (!w25q256jv_waitForReady(fdevice)) {
        return 0;
    }

    bool status = quadSpiReceive4LINESWithAddressAndAlternate4LINES(W25Q256JV_INSTRUCTION_FAST_READ_QUAD_INOUT_4B_ADDR, 4, address, W25Q256JV_ADDRESS_BITS, 0xFF, 8, buffer, length);

    w25q256jv_setTimeout(fdevice, W25Q256JV_TIMEOUT_PAGE_READ_MS);

    if (!status) {
        return 0;
    }

    return length;
}

const flashGeometry_t* w25q256jv_getGeometry(flashDevice_t *fdevice)
{
    return &fdevice->geometry;
}

const flashVTable_t w25q256jv_vTable = {
    .configure = w25q256jv_configure,
    .isReady = w25q256jv_isReady,
    .waitForReady = w25q256jv_waitForReady,
    .eraseSector = w25q256jv_eraseSector,
    .eraseCompletely = w25q256jv_eraseCompletely,
    .pageProgramBegin = w25q256jv_pageProgramBegin,
    .pageProgramContinue = w25q256jv_pageProgramContinue,
    .pageProgramFinish = w25q256jv_pageProgramFinish,
    .pageProgram = w25q256jv_pageProgram,
    .flush = w25q256jv_flush,
    .readBytes = w25q256jv_readBytes,
    .getGeometry = w25q256jv_getGeometry,
};

static void w25q256jv_deviceInit(flashDevice_t *fdevice)
{
    UNUSED(fdevice);
}

bool w25q256jv_init(flashDevice_t *fdevice)
{
    return w25q256jv_identify(fdevice);
}