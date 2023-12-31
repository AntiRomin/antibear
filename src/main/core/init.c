#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "platform.h"

#include "build/debug.h"

#include "core/init.h"

#include "drivers/bus_quadspi.h"
#include "drivers/flash.h"
#include "drivers/system.h"
#include "drivers/time.h"

uint8_t systemState = SYSTEM_STATE_INITIALISING;

void init(void)
{
    systemInit();

    quadSpiInit();

    flashInit();

    debugInit();

    systemState |= SYSTEM_STATE_READY;
}