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

static uint32_t rxBufferSize;
static uint32_t txBufferSize;
static DMA_RAM volatile uint8_t txBuffer[256];
static DMA_RAM volatile uint8_t rxBuffer[256];
static uint32_t rxBufferHead;
static uint32_t rxBufferTail;
static uint32_t txBufferHead;
static uint32_t txBufferTail;

FAST_IRQ_HANDLER void USART1_IRQHandler(void)
{
    UART_HandleTypeDef *huart = &huart1;
    /* UART in mode Receiver ---------------------------------------------------*/
    if ((__HAL_UART_GET_IT(huart, UART_IT_RXNE) != RESET)) {
        uint8_t rbyte = (uint8_t)(huart->Instance->RDR & (uint8_t)0xff);

        rxBuffer[rxBufferHead] = rbyte;
        rxBufferHead = (rxBufferHead + 1) % rxBufferSize;
        CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);

        /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
        CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

        __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
    }

    /* UART parity error interrupt occurred -------------------------------------*/
    if ((__HAL_UART_GET_IT(huart, UART_IT_PE) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_PEF);
    }

    /* UART frame error interrupt occurred --------------------------------------*/
    if ((__HAL_UART_GET_IT(huart, UART_IT_FE) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_FEF);
    }

    /* UART noise error interrupt occurred --------------------------------------*/
    if ((__HAL_UART_GET_IT(huart, UART_IT_NE) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_NEF);
    }

    /* UART Over-Run interrupt occurred -----------------------------------------*/
    if ((__HAL_UART_GET_IT(huart, UART_IT_ORE) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_OREF);
    }

    // UART transmission completed
    if ((__HAL_UART_GET_IT(huart, UART_IT_TC) != RESET)) {
        __HAL_UART_CLEAR_IT(huart, UART_CLEAR_TCF);

        // Disable the UART transmitter
        CLEAR_BIT(huart->Instance->CR1, USART_CR1_TE);
    }

    if ((__HAL_UART_GET_IT(huart, UART_IT_TXE) != RESET)) {
        /* Check that a Tx process is ongoing */
        if (txBufferTail == txBufferHead) {
            /* Disable the UART Transmit Data Register Empty Interrupt */
            CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
        } else {
            if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE)) {
                huart->Instance->TDR = (((uint16_t)txBuffer[txBufferTail]) & (uint16_t)0x01FFU);
            } else {
                huart->Instance->TDR = (uint8_t)(txBuffer[txBufferTail]);
            }
            txBufferTail = (txBufferTail + 1) % txBufferSize;
            SET_BIT(huart1.Instance->CR1, USART_CR1_TE);
        }
    }

    // UART reception idle detected

    if (__HAL_UART_GET_IT(huart, UART_IT_IDLE)) {
        __HAL_UART_CLEAR_IDLEFLAG(huart);
    }

}

static void debugHardwareInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*##-1- Enable GPIO Clocks #################################################*/
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART1 TX GPIO pin configuration */
    GPIO_InitStruct.Pin       = DEBUG_TX_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(DEBUG_TX_GPIO_Port, &GPIO_InitStruct);

    /* UART1 RX GPIO pin configuration */
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

    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    // Receive IRQ
    /* Enable the UART Parity Error Interrupt */
    SET_BIT(huart1.Instance->CR1, USART_CR1_PEIE);

    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    SET_BIT(huart1.Instance->CR3, USART_CR3_EIE);

    /* Enable the UART Data Register not empty Interrupt */
    SET_BIT(huart1.Instance->CR1, USART_CR1_RXNEIE);

    /* Enable Idle Line detection */
    SET_BIT(huart1.Instance->CR1, USART_CR1_IDLEIE);

    // Transmit IRQ
    /* Enable the UART Transmit Data Register Empty Interrupt */
    SET_BIT(huart1.Instance->CR1, USART_CR1_TXEIE);
    SET_BIT(huart1.Instance->CR1, USART_CR1_TCIE);
}

void debugInit(void)
{
    rxBufferSize = 256;
    txBufferSize = 256;
    rxBufferHead = 0;
    rxBufferTail = 0;
    txBufferHead = 0;
    txBufferTail = 0;

    debugHardwareInit();
}

uint32_t debugTotalRxBytesWaiting(void)
{
    if (rxBufferHead >= rxBufferTail) {
        return rxBufferHead - rxBufferTail;
    } else {
        return rxBufferSize + rxBufferHead - rxBufferTail;
    }
}

uint32_t debugTotalTxBytesFree(void)
{
    uint32_t bytesUsed;

    if (txBufferHead >= txBufferTail) {
        bytesUsed = txBufferHead - txBufferTail;
    } else {
        bytesUsed = txBufferSize + txBufferHead - txBufferTail;
    }

    return (txBufferSize - 1) - bytesUsed;
}

bool debugIsTxBufferEmpty(void)
{
    return txBufferTail == txBufferHead;
}

uint8_t debugRead(void)
{
    uint8_t ch;

    ch = rxBuffer[rxBufferTail];
    if (rxBufferTail + 1 >= rxBufferSize) {
        rxBufferTail = 0;
    } else {
        rxBufferTail++;
    }

    return ch;
}

void debugWrite(uint8_t ch)
{
    // Enable the UART transmitter
    SET_BIT(huart1.Instance->CR1, USART_CR1_TE);

    txBuffer[txBufferHead] = ch;

    if (txBufferHead + 1 >= txBufferSize) {
        txBufferHead = 0;
    } else {
        txBufferHead++;
    }

    __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
}

void debugPrint(const char *str)
{
    uint8_t ch;
    while ((ch = *(str++)) != 0) {
        debugWrite(ch);
    }
}

void debugPrintCount(const char *str, size_t count)
{
    uint8_t ch;
    while ((count--) != 0) {
        ch = *(str++);
        debugWrite(ch);
    }
}

int _write(int fd, const void *buf, size_t count)
{
    UNUSED(fd);
    // HAL_UART_Transmit(&huart1, (uint8_t *)buf, count, HAL_MAX_DELAY);
    debugPrintCount(buf, count);
    return count;
}