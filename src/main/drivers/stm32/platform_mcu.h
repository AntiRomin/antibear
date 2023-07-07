#pragma once

#if defined(STM32H743xx)
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "system_stm32h7xx.h"

// Chip Unique ID on H7
#define U_ID_0 (*(uint32_t*)UID_BASE)
#define U_ID_1 (*(uint32_t*)(UID_BASE + 4))
#define U_ID_2 (*(uint32_t*)(UID_BASE + 8))

#define USE_PIN_AF

#ifndef STM32H7
#define STM32H7
#endif

#endif