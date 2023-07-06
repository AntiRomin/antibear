#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#include "drivers/system.h"
#include "drivers/time.h"

#include "core/init.h"

void init(void)
{
    systemInit();
}