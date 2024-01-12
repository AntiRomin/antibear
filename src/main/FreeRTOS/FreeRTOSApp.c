#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

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
        printf("Timebase Tick ms:%lu, us:%lu\n", millis(), micros());
        vTaskDelay(1);
    }
}
#endif

void app_main(void)
{
#if defined(DEBUG)
    printf("Initializing debug task\n");
    xTaskCreate ( prvDebugTask, "DEBUG", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
#endif

    printf("Initializing FreeRTOS task\n");

    printf("FreeRTOS running\n");
    vTaskStartScheduler();

    for( ;; );
}