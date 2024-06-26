#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "common/time.h"
#include "cli/cli.h"
#include "drivers/time.h"

#if defined(DEBUG)
static void prvDebugTask( void * pvParameters )
{
    bool isRunOnlyOneTimes = false;

    ( void ) pvParameters;

    for ( ;; )
    {
        if ( isRunOnlyOneTimes )
        {
            isRunOnlyOneTimes = true;
        }
        // printf("Timebase Tick ms:%lu, us:%lu\n", millis(), micros());
        char buf[30] = {0};
        dateTime_t dateTime = {0};
        rtcGetDateTime(&dateTime);
        dateTimeFormatLocal(buf, &dateTime);
        // printf("UTC:%s\n", buf);
        vTaskDelay(10);
    }
}
#endif

static void prvCliTask( void * pvParameters )
{
    for ( ;; )
    {
        cliProcess();
        vTaskDelay(10);
    }
}

void app_main(void)
{
#if defined(DEBUG)
    printf("Initializing debug task\n");
    xTaskCreate ( prvDebugTask, "DEBUG", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
#endif

    printf("Initializing cli task\n");
    xTaskCreate ( prvCliTask, "CLI", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    printf("Initializing FreeRTOS task\n");

    printf("FreeRTOS running\n");
    vTaskStartScheduler();

    for( ;; );
}