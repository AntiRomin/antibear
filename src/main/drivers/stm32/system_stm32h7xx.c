#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "drivers/system.h"

static void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    // Disable the MPU
    HAL_MPU_Disable();

    // Configure the MPU
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x00;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // Enable the MPU
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void systemInit(void)
{
    // Configure the MPU attributes
    MPU_Config();

    // Enable the CPU Cache
    SCB_EnableICache();
    SCB_EnableDCache();

    // Init cycle counter
    cycleCounterInit();
}

void systemReset(void)
{
    __disable_irq();
    NVIC_SystemReset();
}