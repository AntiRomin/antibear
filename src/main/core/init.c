#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "platform.h"

#include "build/debug.h"

#include "core/init.h"

#include "drivers/bus_i2c.h"
#include "drivers/bus_quadspi.h"
#include "drivers/flash.h"
#include "drivers/rtc.h"
#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/stm32/usb/usbd_device.h"

#include "FatFs/FatFs_app.h"

uint8_t systemState = SYSTEM_STATE_INITIALISING;

void init(void)
{
    systemInit();

    debugInit();

    rtcInit();

    i2cInit();

    quadSpiInit();

    flashInit();

    usbStart();

    FatFs_appInit();

    systemState |= SYSTEM_STATE_READY;
}