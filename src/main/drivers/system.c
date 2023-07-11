#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "system.h"

// cycles per microsecond
static uint32_t usTicks = 0;
// current uptime for 1kHz systick timer. will rollover after 49 days. hopefully we won't care.
static volatile uint32_t sysTickUptime = 0;
static volatile uint32_t sysTickValStamp = 0;
static uint32_t cpuClockFrequency = 0;

void cycleCounterInit(void)
{
    cpuClockFrequency = HAL_RCC_GetSysClockFreq();

    usTicks = cpuClockFrequency / 1000000;
}

// SysTick

static volatile int sysTickPending = 0;

void SysTick_Handler(void)
{
    {
        sysTickUptime++;
        sysTickValStamp = SysTick->VAL;
        sysTickPending = 0;
        (void)(SysTick->CTRL);
    }
    // used by the HAL for some timekeeping and timeouts, should always be 1ms
    HAL_IncTick();
}

// Return system uptime in microseconds (rollover in 70minutes)

uint32_t microsISR(void)
{
    register uint32_t ms, pending, cycle_cnt;

    {
        cycle_cnt = SysTick->VAL;

        if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
            // Update pending.
            // Record it for multiple calls within the same rollover period
            // (Will be cleared when serviced).
            // Note that multiple rollovers are not considered.

            sysTickPending = 1;

            // Read VAL again to ensure the value is read after the rollover.

            cycle_cnt = SysTick->VAL;
        }

        ms = sysTickUptime;
        pending = sysTickPending;
    }

    return ((ms + pending) * 1000) + (usTicks * 1000 - cycle_cnt) / usTicks;
}

uint32_t micros(void)
{
    register uint32_t ms, cycle_cnt;

    // Call microsISR() in interrupt and elevated (non-zero) BASEPRI context

    if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) || (__get_BASEPRI())) {
        return microsISR();
    }

    do {
        ms = sysTickUptime;
        cycle_cnt = SysTick->VAL;
    } while (ms != sysTickUptime || cycle_cnt > sysTickValStamp);

    return (ms * 1000) + (usTicks * 1000 - cycle_cnt) / usTicks;
}

// Return system uptime in milliseconds (rollover in 49 days)
uint32_t millis(void)
{
    return sysTickUptime;
}

#if 1
void delayMicroseconds(uint32_t us)
{
    uint32_t now = micros();
    while (micros() - now < us);
}
#else
void delayMicroseconds(uint32_t us)
{
    uint32_t elapsed = 0;
    uint32_t lastCount = SysTick->VAL;

    for (;;) {
        register uint32_t current_count = SysTick->VAL;
        uint32_t elapsed_us;

        // measure the time elapsed since the last time we checked
        elapsed += current_count - lastCount;
        lastCount = current_count;

        // convert to microseconds
        elapsed_us = elapsed / usTicks;
        if (elapsed_us >= us)
            break;

        // reduce the delay by the elapsed time
        us -= elapsed_us;

        // keep fractional microseconds for the next iteration
        elapsed %= usTicks;
    }
}
#endif

void delay(uint32_t ms)
{
    while (ms--)
        delayMicroseconds(1000);
}

void memProtReset(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    /* Disable the MPU */
    HAL_MPU_Disable();

    // Disable existing regions

    for (uint8_t region = 0; region <= MAX_MPU_REGIONS; region++) {
        MPU_InitStruct.Enable = MPU_REGION_DISABLE;
        MPU_InitStruct.Number = region;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);
    }

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void memProtConfigure(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    // Disable the MPU
    HAL_MPU_Disable();

    // Configure the MPU

    // Setup common members
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;

    MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress      = 0x00000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_64KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO_URO;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    extern uint8_t dmaram_start;
    extern uint8_t dmaram_end;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
    MPU_InitStruct.BaseAddress      = (uint32_t)&dmaram_start;
    {
        uint32_t start = (uint32_t)&dmaram_start & ~0x1F;
        uint32_t length = (uint32_t)&dmaram_end - start;

        if (length < 32) {
            length = 32;
        }

        int msbpos = flsl(length) - 1;

        if (length != (1U <<msbpos)) {
            msbpos += 1;
        }

        MPU_InitStruct.Size         = msbpos;
    }
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    extern uint8_t dmarwaxi_start;
    extern uint8_t dmarwaxi_end;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
    MPU_InitStruct.BaseAddress      = (uint32_t)&dmarwaxi_start;
    {
        uint32_t start = (uint32_t)&dmarwaxi_start & ~0x1F;
        uint32_t length = (uint32_t)&dmarwaxi_end - start;

        if (length < 32) {
            length = 32;
        }

        int msbpos = flsl(length) - 1;

        if (length != (1U <<msbpos)) {
            msbpos += 1;
        }

        MPU_InitStruct.Size         = msbpos;
    }
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    extern uint8_t dmavran_start;
    extern uint8_t dmavram_end;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
    MPU_InitStruct.BaseAddress      = (uint32_t)&dmavran_start;
    {
        uint32_t start = (uint32_t)&dmavran_start & ~0x1F;
        uint32_t length = (uint32_t)&dmavram_end - start;

        if (length < 32) {
            length = 32;
        }

        int msbpos = flsl(length) - 1;

        if (length != (1U <<msbpos)) {
            msbpos += 1;
        }

        MPU_InitStruct.Size         = msbpos;
    }
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // Enable the MPU
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void initialiseMemorySections(void)
{
    /* Load fast-functions into ITCM RAM */
    extern uint8_t tcm_code_start;
    extern uint8_t tcm_code_end;
    extern uint8_t tcm_code;
    memcpy(&tcm_code_start, &tcm_code, (size_t) (&tcm_code_end - &tcm_code_start));

    /* Load FAST_DATA variable initializers into DTCM RAM */
    extern uint8_t _sfastram_data;
    extern uint8_t _efastram_data;
    extern uint8_t _sfastram_idata;
    memcpy(&_sfastram_data, &_sfastram_idata, (size_t) (&_efastram_data - &_sfastram_data));

    /* Load DMA_DATA variable intializers into D1 RAM */
    extern uint8_t _sdmaram_bss;
    extern uint8_t _edmaram_bss;
    extern uint8_t _sdmaram_data;
    extern uint8_t _edmaram_data;
    extern uint8_t _sdmaram_idata;
    bzero(&_sdmaram_bss, (size_t) (&_edmaram_bss - &_sdmaram_bss));
    memcpy(&_sdmaram_data, &_sdmaram_idata, (size_t) (&_edmaram_data - &_sdmaram_data));

    /* Load SDRAM2_DATA variable initializers into SDRAM Bank2 */
    extern uint8_t _ssdram2_bss;
    extern uint8_t _esdram2_bss;
    extern uint8_t _ssdram2_data;
    extern uint8_t _esdram2_data;
    extern uint8_t _ssdram2_idata;
    bzero(&_ssdram2_bss, (size_t) (&_esdram2_bss - &_ssdram2_bss));
    memcpy(&_ssdram2_data, &_ssdram2_idata, (size_t) (&_esdram2_data - &_ssdram2_data));
}