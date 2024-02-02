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

#define I2C_DEFAULT_CLOCKSPEED      800

I2C_HandleTypeDef hi2c1 = {0};

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
}