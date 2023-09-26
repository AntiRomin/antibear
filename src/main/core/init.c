#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "platform.h"

#include "cli/debug.h"

#include "drivers/system.h"
#include "drivers/time.h"
#include "drivers/flash_w25q256jv.h"

#include "drivers/stm32/bus_quadspi.h"

#include "core/init.h"

void init(void)
{
    systemInit();

    debugInit();
    printf("System initailizing...\n");

    quadSpiInit();
    printf("Quad spi initailized\n");

    w25q256jv_init();
    printf("External flash initailized\n");
}