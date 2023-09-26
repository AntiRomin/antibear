#pragma once

/* Definition for USART1 Pins */
#define UART1_TX_PIN                GPIO_PIN_9
#define UART1_TX_GPIO_PORT          GPIOA
#define UART1_RX_PIN                GPIO_PIN_10
#define UART1_RX_GPIO_PORT          GPIOA

void debugInit(void);