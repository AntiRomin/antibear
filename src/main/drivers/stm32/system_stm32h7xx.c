#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "drivers/system.h"

void systemInit(void)
{
    __HAL_RCC_D2SRAM1_CLK_ENABLE();
    __HAL_RCC_D2SRAM2_CLK_ENABLE();
    __HAL_RCC_D2SRAM3_CLK_ENABLE();

    // Init cycle counter
    cycleCounterInit();
}

void systemReset(void)
{
    SCB_DisableDCache();
    SCB_DisableICache();

    __disable_irq();
    NVIC_SystemReset();
}