#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

void vAssertCalled(uint32_t ulLine, const char *pcFile)
{
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    /* Called if an assertion passed to configASSERT() fails.  See
    http://www.freertos.org/a00110.html#configASSERT for more information. */

    printf("ASSERT! Line %d, file %s\r\n", (int)ulLine, pcFile);

    taskENTER_CRITICAL();
    {
        /* You can step out of this function to debug the assertion by using
        the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
        value. */
        while (ulSetToNonZeroInDebuggerToContinue == 0)
        {
            __asm volatile("NOP");
            __asm volatile("NOP");
        }
    }
    taskEXIT_CRITICAL();
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pxTask;

    printf("Stack Over flow! TaskName %s\r\n", pcTaskName);

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}