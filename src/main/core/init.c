#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#include "drivers/system.h"
#include "drivers/time.h"
#include "drivers/flash_w25q256jv.h"

#include "drivers/stm32/bus_quadspi.h"

#include "core/init.h"

void init(void)
{
    systemInit();

    // TODO: Setup Debug serial
    quadSpiInit();

    w25q256jv_init();
}