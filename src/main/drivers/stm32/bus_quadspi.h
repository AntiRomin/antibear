#pragma once

/* Definition for QSPI Pins */
#define QSPI_CLK_PIN                GPIO_PIN_2
#define QSPI_CLK_GPIO_PORT          GPIOB
#define QSPI_CS_PIN                 GPIO_PIN_6
#define QSPI_CS_GPIO_PORT           GPIOG
/* Bank 1 */
#define QSPI_BK1_D0_PIN             GPIO_PIN_8
#define QSPI_BK1_D0_GPIO_PORT       GPIOF
#define QSPI_BK1_D1_PIN             GPIO_PIN_9
#define QSPI_BK1_D1_GPIO_PORT       GPIOF
#define QSPI_BK1_D2_PIN             GPIO_PIN_7
#define QSPI_BK1_D2_GPIO_PORT       GPIOF
#define QSPI_BK1_D3_PIN             GPIO_PIN_6
#define QSPI_BK1_D3_GPIO_PORT       GPIOF
/* Bank 2 */
#define QSPI_BK2_D0_PIN             GPIO_PIN_2
#define QSPI_BK2_D0_GPIO_PORT       GPIOH
#define QSPI_BK2_D1_PIN             GPIO_PIN_3
#define QSPI_BK2_D1_GPIO_PORT       GPIOH
#define QSPI_BK2_D2_PIN             GPIO_PIN_9
#define QSPI_BK2_D2_GPIO_PORT       GPIOG
#define QSPI_BK2_D3_PIN             GPIO_PIN_14
#define QSPI_BK2_D3_GPIO_PORT       GPIOG

extern QSPI_HandleTypeDef hqspi;

void quadSpiInit(void);

bool quadSpiTransmit1LINE(uint8_t instruction, uint8_t dummyCycles, const uint8_t *out, int length);
bool quadSpiReceive1LINE(uint8_t instruction, uint8_t dummyCycles, uint8_t *in, int length);
bool quadSpiReceiveWithAddress1LINE(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint8_t *in, int length);
bool quadSpiReceiveWithAddress4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint8_t *in, int length);
bool quadSpiReceiveWith4LINESAddress4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint8_t *in, int length);
bool quadSpiReceiveWith4LINESAddressAndAlternate4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint32_t alternate, uint8_t alternateSize, uint8_t *in, int length);
bool quadSpiTransmitWithAddress1LINE(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, const uint8_t *out, int length);
bool quadSpiTransmitWithAddress4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, const uint8_t *out, int length);
bool quadSpiInstructionWithAddress1LINE(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize);
bool quadSpiInstructionWithData1LINE(uint8_t instruction, uint8_t dummyCycles, const uint8_t *out, int length);