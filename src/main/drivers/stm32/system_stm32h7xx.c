#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "drivers/system.h"

void SystemClock_Config(void);

void systemInit(void)
{
    // Init cycle counter
    cycleCounterInit();

    // SysTick is updated whenever HAL_RCC_ClockConfig is called.
}

void systemReset(void)
{
    __disable_irq();
    NVIC_SystemReset();
}