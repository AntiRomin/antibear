#pragma once

#include <stdint.h>

void w25q256jv_init(void);

bool w25q256jv_isReady(void);

int w25q256jv_writeBytes(uint32_t address, const uint8_t *data, uint32_t length);
int w25q256jv_readBytes(uint32_t address, uint8_t *buffer, uint32_t length);

void w25q256jv_eraseSector(uint32_t address);
void w25q256jv_eraseCompletely(void);