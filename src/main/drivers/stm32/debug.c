#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "platform.h"
#include "build/debug.h"

/* Definition for USART1 Pins */
#define DEBUG_TX_Pin                      GPIO_PIN_9
#define DEBUG_TX_GPIO_Port                GPIOA
#define DEBUG_RX_Pin                      GPIO_PIN_10
#define DEBUG_RX_GPIO_Port                GPIOA

UART_HandleTypeDef huart1;

static void debugHardwareInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*##-1- Enable GPIO Clocks #################################################*/
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART1 TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = DEBUG_TX_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(DEBUG_TX_GPIO_Port, &GPIO_InitStruct);

    /* UART1 RX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = DEBUG_RX_Pin;
    HAL_GPIO_Init(DEBUG_RX_GPIO_Port, &GPIO_InitStruct);

    /*##-3- Enable peripherals Clocks ##########################################*/
    /* Enable the USART1 clock */
    __HAL_RCC_USART1_CLK_ENABLE();
    /* Reset the USART1 */
    __HAL_RCC_USART1_FORCE_RESET();
    __HAL_RCC_USART1_RELEASE_RESET();

    /*##-4- USART1 parameter configuration ####################################*/
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = DEBUG_UART_BAUDRATE;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_8;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	HAL_UART_Init(&huart1);

    HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8);
    HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8);
    HAL_UARTEx_DisableFifoMode(&huart1);
}

void debugInit(void)
{
    debugHardwareInit();
}

int _write(int fd, const void *buf, size_t count)
{
    UNUSED(fd);
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, count, HAL_MAX_DELAY);
    return count;
}