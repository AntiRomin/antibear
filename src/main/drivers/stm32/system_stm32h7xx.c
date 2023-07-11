#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "drivers/system.h"

void systemInit(void)
{
    // Init cycle counter
    cycleCounterInit();
}

void systemReset(void)
{
    __disable_irq();
    NVIC_SystemReset();
}