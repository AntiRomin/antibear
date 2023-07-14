#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "drivers/stm32/bus_quadspi.h"

#define QUADSPI_DEFAULT_TIMEOUT 10

QSPI_HandleTypeDef hqspi;

void quadSpiInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /*##-1- Enable GPIO Clocks #################################################*/
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* QSPI CLK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = QSPI_CLK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI CS GPIO pin configuration  */
    GPIO_InitStruct.Pin       = QSPI_CS_PIN;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_CS_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI D0 GPIO pin configuration  */
    GPIO_InitStruct.Pin       = QSPI_BK1_D0_PIN;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = QSPI_BK2_D0_PIN;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_BK2_D0_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI D1 GPIO pin configuration  */
    GPIO_InitStruct.Pin       = QSPI_BK1_D1_PIN;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = QSPI_BK2_D1_PIN;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_BK2_D1_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI D2 GPIO pin configuration  */
    GPIO_InitStruct.Pin       = QSPI_BK1_D2_PIN;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = QSPI_BK2_D2_PIN;
    HAL_GPIO_Init(QSPI_BK2_D2_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI D3 GPIO pin configuration  */
    GPIO_InitStruct.Pin       = QSPI_BK1_D3_PIN;
    HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = QSPI_BK2_D3_PIN;
    HAL_GPIO_Init(QSPI_BK2_D3_GPIO_PORT, &GPIO_InitStruct);

    /*##-3- Enable peripherals Clocks ##########################################*/
    /* Enable the QuadSPI memory interface clock */
    __HAL_RCC_QSPI_CLK_ENABLE();
    /* Reset the QuadSPI memory interface */
    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();

    /*##-4- QUADSPI parameter configuration ####################################*/
    hqspi.Instance = QUADSPI;
    hqspi.Init.ClockPrescaler = 1;
    hqspi.Init.FifoThreshold = 1;
    hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_6_CYCLE;
    hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
    hqspi.Init.FlashID = QSPI_FLASH_ID_1;

#if !defined(USE_QSPI_DUALFLASH)
    hqspi.Init.FlashSize = 24;
    hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
#else
    hqspi.Init.FlashSize = 25;
    hqspi.Init.DualFlash = QSPI_DUALFLASH_ENABLE;
#endif
	HAL_QSPI_Init(&hqspi);
}

static const uint32_t quadSpi_addressSizeMap[] = {
    QSPI_ADDRESS_8_BITS,
    QSPI_ADDRESS_16_BITS,
    QSPI_ADDRESS_24_BITS,
    QSPI_ADDRESS_32_BITS
};

static uint32_t quadSpi_addressSizeFromValue(uint8_t addressSize)
{
    return quadSpi_addressSizeMap[((addressSize + 1) / 8) - 1]; // rounds to nearest QSPI_ADDRESS_* value that will hold the address.
}

static const uint32_t quadSpi_alternateSizeMap[] = {
    QSPI_ALTERNATE_BYTES_8_BITS,
    QSPI_ALTERNATE_BYTES_16_BITS,
    QSPI_ALTERNATE_BYTES_24_BITS,
    QSPI_ALTERNATE_BYTES_32_BITS
};

static uint32_t quadSpi_alternateSizeFromValue(uint8_t alternateSize)
{
    return quadSpi_alternateSizeMap[((alternateSize + 1) / 8) - 1]; // rounds to nearest QSPI_ADDRESS_* value that will hold the address.
}

bool quadSpiTransmit1LINE(uint8_t instruction, uint8_t dummyCycles, const uint8_t *out, int length)
{
    HAL_StatusTypeDef status;

    QSPI_CommandTypeDef cmd;
    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode       = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_NONE;
    cmd.DummyCycles       = dummyCycles;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction       = instruction;
    cmd.NbData            = length;

    if (out) {
        cmd.DataMode      = QSPI_DATA_1_LINE;
    }

    status = HAL_QSPI_Command(&hqspi, &cmd, QUADSPI_DEFAULT_TIMEOUT);
    bool timeout= (status != HAL_OK);
    if (!timeout) {
        if (out && length > 0) {
            status = HAL_QSPI_Transmit(&hqspi, (uint8_t *)out, QUADSPI_DEFAULT_TIMEOUT);

            timeout = (status != HAL_OK);
        }
    }

    if (timeout) {
        return false;
    }

    return true;
}

bool quadSpiReceive1LINE(uint8_t instruction, uint8_t dummyCycles, uint8_t *in, int length)
{
    HAL_StatusTypeDef status;

    QSPI_CommandTypeDef cmd;
    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode       = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_1_LINE;
    cmd.DummyCycles       = dummyCycles;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction       = instruction;
    cmd.NbData            = length;

    status = HAL_QSPI_Command(&hqspi, &cmd, QUADSPI_DEFAULT_TIMEOUT);
    bool timeout= (status != HAL_OK);
    if (!timeout) {
        status = HAL_QSPI_Receive(&hqspi, (uint8_t *)in, QUADSPI_DEFAULT_TIMEOUT);

        timeout = (status != HAL_OK);
    }

    if (timeout) {
        return false;
    }

    return true;
}

bool quadSpiReceiveWithAddress1LINE(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint8_t *in, int length)
{
    HAL_StatusTypeDef status;

    QSPI_CommandTypeDef cmd;
    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode       = QSPI_ADDRESS_1_LINE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_1_LINE;
    cmd.DummyCycles       = dummyCycles;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction       = instruction;
    cmd.Address           = address;
    cmd.AddressSize       = quadSpi_addressSizeFromValue(addressSize);
    cmd.NbData            = length;

    status = HAL_QSPI_Command(&hqspi, &cmd, QUADSPI_DEFAULT_TIMEOUT);
    bool timeout = (status != HAL_OK);
    if (!timeout) {
        status = HAL_QSPI_Receive(&hqspi, in, QUADSPI_DEFAULT_TIMEOUT);
        timeout = (status != HAL_OK);
    }

    if (timeout) {
        return false;
    }

    return true;
}

bool quadSpiReceiveWithAddress4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint8_t *in, int length)
{
    HAL_StatusTypeDef status;

    QSPI_CommandTypeDef cmd;
    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode       = QSPI_ADDRESS_1_LINE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_4_LINES;
    cmd.DummyCycles       = dummyCycles;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction       = instruction;
    cmd.Address           = address;
    cmd.AddressSize       = quadSpi_addressSizeFromValue(addressSize);
    cmd.NbData            = length;

    status = HAL_QSPI_Command(&hqspi, &cmd, QUADSPI_DEFAULT_TIMEOUT);
    bool timeout = (status != HAL_OK);
    if (!timeout) {
        status = HAL_QSPI_Receive(&hqspi, in, QUADSPI_DEFAULT_TIMEOUT);
        timeout = (status != HAL_OK);
    }

    if (timeout) {
        return false;
    }

    return true;
}

bool quadSpiReceiveWith4LINESAddress4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint8_t *in, int length)
{
    HAL_StatusTypeDef status;

    QSPI_CommandTypeDef cmd;
    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode       = QSPI_ADDRESS_4_LINES;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_4_LINES;
    cmd.DummyCycles       = dummyCycles;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction       = instruction;
    cmd.Address           = address;
    cmd.AddressSize       = quadSpi_addressSizeFromValue(addressSize);
    cmd.NbData            = length;

    status = HAL_QSPI_Command(&hqspi, &cmd, QUADSPI_DEFAULT_TIMEOUT);
    bool timeout = (status != HAL_OK);
    if (!timeout) {
        status = HAL_QSPI_Receive(&hqspi, in, QUADSPI_DEFAULT_TIMEOUT);
        timeout = (status != HAL_OK);
    }

    if (timeout) {
        return false;
    }

    return true;
}

bool quadSpiReceiveWith4LINESAddressAndAlternate4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint32_t alternate, uint8_t alternateSize, uint8_t *in, int length)
{
    HAL_StatusTypeDef status;

    QSPI_CommandTypeDef cmd;
    cmd.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode        = QSPI_ADDRESS_4_LINES;
    cmd.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd.DataMode           = QSPI_DATA_4_LINES;
    cmd.DummyCycles        = dummyCycles;
    cmd.DdrMode            = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction        = instruction;
    cmd.Address            = address;
    cmd.AddressSize        = quadSpi_addressSizeFromValue(addressSize);
    cmd.AlternateBytes     = alternate;
    cmd.AlternateBytesSize = quadSpi_alternateSizeFromValue(alternateSize);
    cmd.NbData             = length;

    status = HAL_QSPI_Command(&hqspi, &cmd, QUADSPI_DEFAULT_TIMEOUT);
    bool timeout = (status != HAL_OK);
    if (!timeout) {
        status = HAL_QSPI_Receive(&hqspi, in, QUADSPI_DEFAULT_TIMEOUT);
        timeout = (status != HAL_OK);
    }

    if (timeout) {
        return false;
    }

    return true;
}

bool quadSpiInstructionWithAddress1LINE(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize)
{
    HAL_StatusTypeDef status;

    QSPI_CommandTypeDef cmd;
    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode       = QSPI_ADDRESS_1_LINE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_NONE;
    cmd.DummyCycles       = dummyCycles;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction       = instruction;
    cmd.Address           = address;
    cmd.AddressSize       = quadSpi_addressSizeFromValue(addressSize);
    cmd.NbData            = 0;

    status = HAL_QSPI_Command(&hqspi, &cmd, QUADSPI_DEFAULT_TIMEOUT);
    bool timeout = (status != HAL_OK);

    if (timeout) {
        return false;
    }

    return true;
}

bool quadSpiInstructionWithData1LINE(uint8_t instruction, uint8_t dummyCycles, const uint8_t *out, int length)
{
    HAL_StatusTypeDef status;

    QSPI_CommandTypeDef cmd;
    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode       = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_1_LINE;
    cmd.DummyCycles       = dummyCycles;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction       = instruction;
    cmd.NbData            = length;

    status = HAL_QSPI_Command(&hqspi, &cmd, QUADSPI_DEFAULT_TIMEOUT);
    bool timeout =(status != HAL_OK);

    if (!timeout) {
        status = HAL_QSPI_Transmit(&hqspi, (uint8_t *)out, QUADSPI_DEFAULT_TIMEOUT);
        timeout = (status != HAL_OK);
    }

    if (timeout) {
        return false;
    }

    return true;
}