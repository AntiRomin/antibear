#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "drivers/bus_i2c.h"
#include "drivers/bus_i2c_timing.h"

/* Definition for I2C Pins */
#define I2C1_SDA_PIN                GPIO_PIN_9
#define I2C1_SDA_GPIO_Port          GPIOB
#define I2C1_SCL_PIN                GPIO_PIN_8
#define I2C1_SCL_GPIO_Port          GPIOB

#define I2C_DEFAULT_CLOCKSPEED      400

I2C_HandleTypeDef hi2c1 = {0};

void I2C1_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&hi2c1);
}

void I2C1_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&hi2c1);
}

void i2cInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    uint32_t i2cPclk = 0;

    /*##-1- Enable GPIO Clocks #################################################*/
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* I2C1 SDA GPIO pin configuration  */
    GPIO_InitStruct.Pin       = I2C1_SDA_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(I2C1_SDA_GPIO_Port, &GPIO_InitStruct);

    /* I2C1 SCL GPIO pin configuration  */
    GPIO_InitStruct.Pin       = I2C1_SCL_PIN;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(I2C1_SCL_GPIO_Port, &GPIO_InitStruct);

    /*##-3- Enable peripherals Clocks ##########################################*/
    /* Enable the I2C memory interface clock */
    __HAL_RCC_I2C1_CLK_ENABLE();
    /* Reset the I2C memory interface */
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();

    /*##-4- I2C parameter configuration ####################################*/
    hi2c1.Instance = I2C1;

    i2cPclk = HAL_RCC_GetPCLK1Freq();
    hi2c1.Init.Timing = i2cClockTIMINGR(i2cPclk, I2C_DEFAULT_CLOCKSPEED, 0);

    hi2c1.Init.OwnAddress1 = 0x0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0x0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c1);

    // Enable the Analog I2C Filter
    HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);
    HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);
    HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);

    // Setup interrupt handlers
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
}

// Blocking write
static bool i2cWrite(I2C_HandleTypeDef *pHandle, uint8_t addr_, uint8_t reg_, uint8_t data)
{
    if (!pHandle->Instance) {
        return false;
    }

    HAL_StatusTypeDef status;

    if (reg_ == 0xFF)
        status = HAL_I2C_Master_Transmit(pHandle, addr_ << 1, &data, 1, I2C_TIMEOUT_SYS_TICKS);
    else
        status = HAL_I2C_Mem_Write(pHandle, addr_ << 1, reg_, I2C_MEMADD_SIZE_8BIT, &data, 1, I2C_TIMEOUT_SYS_TICKS);

    if (status != HAL_OK) {
        return false;
    }

    return true;
}

// Non-blocking write
static bool i2cWriteBuffer(I2C_HandleTypeDef *pHandle, uint8_t addr_, uint8_t reg_, uint8_t len_, uint8_t *data)
{
    if (!pHandle->Instance) {
        return false;
    }

    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Write_IT(pHandle, addr_ << 1, reg_, I2C_MEMADD_SIZE_8BIT, data, len_);

    if (status == HAL_BUSY) {
        return false;
    }

    if (status != HAL_OK) {
        return false;
    }

    return true;
}

// Blocking read
static bool i2cRead(I2C_HandleTypeDef *pHandle, uint8_t addr_, uint8_t reg_, uint8_t len, uint8_t* buf)
{
    if (!pHandle->Instance) {
        return false;
    }

    HAL_StatusTypeDef status;

    if (reg_ == 0xFF)
        status = HAL_I2C_Master_Receive(pHandle, addr_ << 1, buf, len, I2C_TIMEOUT_SYS_TICKS);
    else
        status = HAL_I2C_Mem_Read(pHandle, addr_ << 1, reg_, I2C_MEMADD_SIZE_8BIT, buf, len, I2C_TIMEOUT_SYS_TICKS);

    if (status != HAL_OK) {
        return false;
    }

    return true;
}

// Non-blocking read
static bool i2cReadBuffer(I2C_HandleTypeDef *pHandle, uint8_t addr_, uint8_t reg_, uint8_t len, uint8_t *buf)
{
    if (!pHandle->Instance) {
        return false;
    }

    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read_IT(pHandle, addr_ << 1, reg_, I2C_MEMADD_SIZE_8BIT, buf, len);

    if (status == HAL_BUSY) {
        return false;
    }

    if (status != HAL_OK) {
        return false;
    }

    return true;
}

static bool i2cBusy(I2C_HandleTypeDef *pHandle, bool *error)
{
    if (error) {
        *error = pHandle->ErrorCode;
    }

    if (pHandle->State == HAL_I2C_STATE_READY) {
        if (__HAL_I2C_GET_FLAG(pHandle, I2C_FLAG_BUSY) == SET)
        {
            return true;
        }

        return false;
    }

    return true;
}

bool i2cBusWriteRegister(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg, uint8_t data)
{
    return i2cWrite(pHandle, addr, reg, data);
}

bool i2cBusWriteRegisterStart(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg, uint8_t data)
{
    // Need a static value, not on the stack
    static uint8_t byte;

    byte = data;

    return i2cWriteBuffer(pHandle, addr, reg, sizeof(byte), &byte);
}

bool i2cBusWriteRegisterBufferStart(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg, uint8_t *data, uint8_t length)
{
    return i2cWriteBuffer(pHandle, addr, reg, length, data);
}

bool i2cBusReadRegisterBuffer(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg, uint8_t *data, uint8_t length)
{
    return i2cRead(pHandle, addr, reg, length, data);
}

uint8_t i2cBusReadRegister(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg)
{
    uint8_t data;
    i2cRead(pHandle, addr, reg, 1, &data);
    return data;
}

bool i2cBusReadRegisterBufferStart(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg, uint8_t *data, uint8_t length)
{
    return i2cReadBuffer(pHandle, addr, reg, length, data);
}

bool i2cBusBusy(I2C_HandleTypeDef *pHandle, bool *error)
{
    return i2cBusy(pHandle, error);
}