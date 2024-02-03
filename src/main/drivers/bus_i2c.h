#pragma once

#include "platform.h"

#define I2C_TIMEOUT_US          10000
#define I2C_TIMEOUT_SYS_TICKS   (I2C_TIMEOUT_US / 1000)

extern I2C_HandleTypeDef hi2c1;

void i2cInit(void);

bool i2cBusWriteRegister(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg, uint8_t data);
bool i2cBusWriteRegisterStart(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg, uint8_t data);
bool i2cBusWriteRegisterBufferStart(I2C_HandleTypeDef *pHandle, uint8_t addre, uint8_t reg, uint8_t *data, uint8_t length);
bool i2cBusReadRegisterBuffer(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg, uint8_t *data, uint8_t length);
uint8_t i2cBusReadRegister(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg);
bool i2cBusReadRegisterBufferStart(I2C_HandleTypeDef *pHandle, uint8_t addr, uint8_t reg, uint8_t *data, uint8_t length);
bool i2cBusBusy(I2C_HandleTypeDef *pHandle, bool *error);