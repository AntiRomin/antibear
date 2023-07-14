#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/system.h"
#include "drivers/flash_w25q256jv.h"
#include "drivers/stm32/bus_quadspi.h"

// JEDEC ID
#define JEDEC_ID_WINBOND_W25Q256JV              0xEF4019

// Device size parameters
#define W25Q256JV_PAGE_SIZE         2048
#define W25Q256JV_PAGES_PER_BLOCK   64
#define W25Q256JV_BLOCKS_PER_DIE    2048
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
#define W25Q256JV_TIMEOUT_BLOCK_ERASE_64KB_MS   2000        // tBE2max = 2000ms, tBE2typ = 150ms
#define W25Q256JV_TIMEOUT_CHIP_ERASE_MS        (400 * 1000) // tCEmax 400s, tCEtyp = 80s

#define W25Q256JV_TIMEOUT_PAGE_PROGRAM_MS       3           // tPPmax = 3ms, tPPtyp = 0.7ms
#define W25Q256JV_TIMEOUT_WRITE_ENABLE_MS       1

uint32_t timeoutAt = 0;

static bool w25q256jv_waitForReady(void);
static void w25q256jv_waitForTimeout(void);

static void w25q256jv_setTimeout(timeMs_t timeoutMillis)
{
    timeMs_t nowMs = microsISR() / 1000;
    timeoutAt = nowMs + timeoutMillis;
}

static void w25q256jv_performOneByteCommand(uint8_t command)
{
    quadSpiTransmit1LINE(command, 0, NULL, 0);
}

static void w25q256jv_performCommandWithAddress(uint8_t command, uint32_t address)
{
    quadSpiInstructionWithAddress1LINE(command, 0, address, W25Q256JV_ADDRESS_BITS);
}

static void w25q256jv_writeEnable(void)
{
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

static void w25q256jv_deviceReset(void)
{
    w25q256jv_waitForReady();
    w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_ENABLE_RESET);
    w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_RESET_DEVICE);

    w25q256jv_setTimeout(W25Q256JV_TIMEOUT_RESET_MS);
    w25q256jv_waitForTimeout();

    w25q256jv_waitForReady();

    uint16_t registerValue;

    registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS1_REG);

#if !defined(USE_QSPI_DUALFLASH)
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR1_BITS_BLOCK_PROTECT) != 0) {
#else
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR1_BITS_BLOCK_PROTECT) != 0 ||
        (((uint8_t *)&registerValue)[1] & W25Q256JV_SR1_BITS_BLOCK_PROTECT) != 0) {
#endif
        // Enable QUADSPI mode.
        registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS1_REG);

        uint16_t newValue = registerValue;
#if !defined(USE_QSPI_DUALFLASH)
        ((uint8_t *)&newValue)[0] &= ~W25Q256JV_SR1_BITS_BLOCK_PROTECT;
#else
        ((uint8_t *)&newValue)[0] &= ~W25Q256JV_SR1_BITS_BLOCK_PROTECT;
        ((uint8_t *)&newValue)[1] &= ~W25Q256JV_SR1_BITS_BLOCK_PROTECT;
#endif

        w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_WRITE_ENABLE);
        // w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_VOLATILE_WRITE_ENABLE);
        w25q256jv_writeRegister(W25Q256JV_INSTRUCTION_WRITE_STATUS1_REG, newValue);
    }

    registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS2_REG);

#if !defined(USE_QSPI_DUALFLASH)
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR2_BIT_QUAD_ENABLE) == 0) {
#else
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR2_BIT_QUAD_ENABLE) == 0 ||
        (((uint8_t *)&registerValue)[1] & W25Q256JV_SR2_BIT_QUAD_ENABLE) == 0) {
#endif
        // Enable QUADSPI mode.
        registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS2_REG);

        uint16_t newValue = registerValue;
#if !defined(USE_QSPI_DUALFLASH)
        ((uint8_t *)&newValue)[0] |= W25Q256JV_SR2_BIT_QUAD_ENABLE;
#else
        ((uint8_t *)&newValue)[0] |= W25Q256JV_SR2_BIT_QUAD_ENABLE;
        ((uint8_t *)&newValue)[1] |= W25Q256JV_SR2_BIT_QUAD_ENABLE;
#endif

        w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_WRITE_ENABLE);
        // w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_VOLATILE_WRITE_ENABLE);
        w25q256jv_writeRegister(W25Q256JV_INSTRUCTION_WRITE_STATUS2_REG, newValue);
    }

    registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS3_REG);

#if !defined(USE_QSPI_DUALFLASH)
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE) == 0 ) {
#else
    if ((((uint8_t *)&registerValue)[0] & W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE) == 0 ||
        (((uint8_t *)&registerValue)[1] & W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE) == 0) {
#endif
        // Enable QUADSPI mode.
        registerValue = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS3_REG);

        uint16_t newValue = registerValue;
#if !defined(USE_QSPI_DUALFLASH)
        ((uint8_t *)&newValue)[0] |= W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE;
#else
        ((uint8_t *)&newValue)[0] |= W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE;
        ((uint8_t *)&newValue)[1] |= W25Q256JV_SR3_BIT_4B_ADDRESS_ENABLE;
#endif

        w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_WRITE_ENABLE);
        // w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_VOLATILE_WRITE_ENABLE);
        w25q256jv_writeRegister(W25Q256JV_INSTRUCTION_WRITE_STATUS3_REG, newValue);
    }
}

bool w25q256jv_isReady(void)
{
    uint16_t status = w25q256jv_readRegister(W25Q256JV_INSTRUCTION_READ_STATUS1_REG);

#if !defined(USE_QSPI_DUALFLASH)
    bool busy = (((uint8_t *)&status)[0] & W25Q256JV_SR1_BIT_WRITE_IN_PROGRESS);
#else
    bool busy = ((((uint8_t *)&status)[0] & W25Q256JV_SR1_BIT_WRITE_IN_PROGRESS) &&
                 (((uint8_t *)&status)[1] & (W25Q256JV_SR1_BIT_WRITE_IN_PROGRESS)));
#endif

    return !busy;
}

bool w25q256jv_hasTimedOut(void)
{
    uint32_t nowMs = microsISR() / 1000;
    if (cmp32(nowMs, timeoutAt) >= 0) {
        return true;
    }
    return false;
}

void w25q256jv_waitForTimeout(void)
{
   while (!w25q256jv_hasTimedOut()) { }

   timeoutAt = 0;
}

bool w25q256jv_waitForReady(void)
{
    bool ready = true;
    while (!w25q256jv_isReady()) {
        if (w25q256jv_hasTimedOut()) {
            ready = false;
            break;
        }
    }
    timeoutAt = 0;

    return ready;
}

static bool w25q256jv_identify(void)
{
    bool isDetected = false;

    uint8_t in[W25Q256JV_JEDEC_ID_LENGTH / 8];

    quadSpiReceive1LINE(W25Q256JV_INSTRUCTION_READ_JEDEC_ID, 0, in, W25Q256JV_JEDEC_ID_LENGTH / 8);

#if !defined(USE_QSPI_DUALFLASH)
    if ((in[0] << 16 | in[1] << 8 | in[2]) == JEDEC_ID_WINBOND_W25Q256JV) {
#else
    if (((in[0] << 16 | in[2] << 8 | in[4]) == JEDEC_ID_WINBOND_W25Q256JV) &&
        ((in[1] << 16 | in[3] << 8 | in[5]) == JEDEC_ID_WINBOND_W25Q256JV)) {
#endif
        isDetected = true;
    }

    return isDetected;
}

void w25q256jv_eraseSector(uint32_t address)
{
    w25q256jv_waitForReady();

    w25q256jv_writeEnable();

    w25q256jv_performCommandWithAddress(W25Q256JV_INSTRUCTION_BLOCK_ERASE_64KB_4B_ADDR, address);
    
    w25q256jv_setTimeout(W25Q256JV_TIMEOUT_BLOCK_ERASE_64KB_MS);
}

void w25q256jv_eraseCompletely(void)
{
    w25q256jv_waitForReady();

    w25q256jv_writeEnable();

    w25q256jv_performOneByteCommand(W25Q256JV_INSTRUCTION_CHIP_ERASE);

    w25q256jv_setTimeout(W25Q256JV_TIMEOUT_CHIP_ERASE_MS);
}

static int w25q256jv_readBytes(uint32_t address, uint8_t *buffer, uint32_t length)
{
    if (!w25q256jv_waitForReady()) {
        return 0;
    }

    bool status = quadSpiReceiveWith4LINESAddressAndAlternate4LINES(W25Q256JV_INSTRUCTION_FAST_READ_QUAD_OUT_4B_ADDR, 24, address, W25Q256JV_ADDRESS_BITS, 0x00000000, 32, buffer, length);

    w25q256jv_setTimeout(W25Q256JV_TIMEOUT_PAGE_READ_MS);

    if (!status) {
        return 0;
    }

    return length;
}

void w25q256jv_configure(void)
{
    w25q256jv_deviceReset();
}

uint16_t status1 = 0;
uint16_t status2 = 0;
uint16_t status3 = 0;
uint8_t DID[2];
uint8_t MDID[4];
uint8_t MDID2[4];
uint8_t JEDEC[6];
uint8_t UID[16];
uint8_t DataBuffer[512];
void flashInit(void)
{
    if (w25q256jv_identify()) {
        w25q256jv_configure();

        status1 = w25q256jv_readRegister(0x05);
        status2 = w25q256jv_readRegister(0x35);
        status3 = w25q256jv_readRegister(0x15);
        quadSpiReceiveWithAddress1LINE(W25Q256JV_INSTRUCTION_READ_DID, 0, 0x00000000, W25Q256JV_ADDRESS_BITS, DID, 2);
        quadSpiReceiveWithAddress1LINE(W25Q256JV_INSTRUCTION_READ_MID_DID, 0, 0x00000000, W25Q256JV_ADDRESS_BITS, MDID, 4);
        quadSpiReceiveWith4LINESAddress4LINES(W25Q256JV_INSTRUCTION_READ_MID_DID_QUAD_INOUT, 6, 0x00000000, W25Q256JV_ADDRESS_BITS, MDID2, 4);
        quadSpiReceiveWithAddress1LINE(W25Q256JV_INSTRUCTION_READ_UID, 8, 0x00000000, W25Q256JV_ADDRESS_BITS, UID, 16);
        w25q256jv_readBytes(0x00000000, DataBuffer, 512);
        __NOP();
    }
}