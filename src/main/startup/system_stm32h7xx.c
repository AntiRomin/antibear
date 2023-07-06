/**
  ******************************************************************************
  * @file    system_stm32h7xx.c
  * @author  MCD Application Team
  * @brief   CMSIS Cortex-M Device Peripheral Access Layer System Source File.
  *
  *   This file provides two functions and one global variable to be called from
  *   user application:
  *      - SystemInit(): This function is called at startup just after reset and
  *                      before branch to main program. This call is made inside
  *                      the "startup_stm32h7xx.s" file.
  *
  *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be used
  *                                  by the user application to setup the SysTick
  *                                  timer or configure other parameters.
  *
  *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
  *                                 be called whenever the core clock is changed
  *                                 during program execution.
  *
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup stm32h7xx_system
  * @{
  */

/** @addtogroup STM32H7xx_System_Private_Includes
  * @{
  */

#include <string.h>

#include "platform.h"

#include "drivers/system.h"

#if !defined  (HSE_VALUE)
  #define HSE_VALUE    ((uint32_t)8000000) /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (CSI_VALUE)
  #define CSI_VALUE    ((uint32_t)4000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* CSI_VALUE */

#if !defined  (HSI_VALUE)
  #define HSI_VALUE    ((uint32_t)64000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

/**
  * @}
  */

/** @addtogroup STM32H7xx_System_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32H7xx_System_Private_Defines
  * @{
  */

/************************* Miscellaneous Configuration ************************/

/*!< Uncomment the following line if you need to relocate your vector Table in
     Internal SRAM. */
/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x00       /*!< Vector Table base offset field.
                                      This value must be a multiple of 0x200. */
/******************************************************************************/

/**
  * @}
  */

/** @addtogroup STM32H7xx_System_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32H7xx_System_Private_Variables
  * @{
  */
  /* This variable is updated in three ways:
      1) by calling CMSIS function SystemCoreClockUpdate()
      2) by calling HAL API function HAL_RCC_GetHCLKFreq()
      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
         Note: If you use this function to configure the system clock; then there
               is no need to call the 2 first functions listed above, since SystemCoreClock
               variable is updated automatically.
  */
  uint32_t SystemCoreClock = 64000000;
  uint32_t SystemD2Clock = 64000000;
  const  uint8_t D1CorePrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};

/**
  * @}
  */

/** @addtogroup STM32H7xx_System_Private_Functions
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32H7xx_System_Private_Functions
  * @{
  */

static void ErrorHandler(void)
{
    while (1);
}

typedef struct pllConfig_s {
    uint16_t clockMhz;
    uint8_t m;
    uint16_t n;
    uint8_t p;
    uint8_t q;
    uint8_t r;
    uint32_t vos;
    uint32_t vciRange;
} pllConfig_t;

#if defined(STM32H743xx) || defined(STM32H750xx)
/*
   PLL1 configuration for different silicon revisions of H743 and H750.

   Note for future overclocking support.

   - Rev.Y (and Rev.X), nominal max at 400MHz, runs stably overclocked to 480MHz.
   - Rev.V, nominal max at 480MHz, runs stably at 540MHz, but not to 600MHz (VCO probably out of operating range)

   - A possible frequency table would look something like this, and a revision
     check logic would place a cap for Rev.Y and V.

        400 420 440 460 (Rev.Y & V ends here) 480 500 520 540
 */

// 400MHz for Rev.Y (and Rev.X)
pllConfig_t pll1ConfigRevY = {
    .clockMhz = 400,
    .m = 4,
    .n = 800000000 / HSE_VALUE * 4, // Holds DIVN's output to DIVP at 800Mhz.
    .p = 2,
    // Dividing PLLQ here by 8 keeps PLLQ1, used for SPI, below 200Mhz, required per the spec.
    .q = 8,
    .r = 5,
    .vos = PWR_REGULATOR_VOLTAGE_SCALE1,
    .vciRange = RCC_PLL1VCIRANGE_2,
};

// 480MHz for Rev.V
pllConfig_t pll1ConfigRevV = {
    .clockMhz = 480,
    .m = 4,
    .n = 960000000 / HSE_VALUE * 4, // Holds DIVN's output to DIVP at 960Mhz.
    .p = 2,
    // Dividing PLLQ here by 8 keeps PLLQ1, used for SPI, below 200Mhz, required per the spec.
    .q = 8,
    .r = 5,
    .vos = PWR_REGULATOR_VOLTAGE_SCALE0,
    .vciRange = RCC_PLL1VCIRANGE_2,
};

#define MCU_HCLK_DIVIDER RCC_HCLK_DIV2

// H743 and H750
// For HCLK=200MHz with VOS1 range, ST recommended flash latency is 2WS.
// RM0433 (Rev.5) Table 12. FLASH recommended number of wait states and programming delay
//
// For higher HCLK frequency, VOS0 is available on RevV silicons, with FLASH wait states 4WS
// AN5312 (Rev.1) Section 1.2.1 Voltage scaling Table.1
//
// XXX Check if Rev.V requires a different value

#define MCU_FLASH_LATENCY FLASH_LATENCY_2

// Source for CRS input
#define MCU_RCC_CRS_SYNC_SOURCE RCC_CRS_SYNC_SOURCE_USB2

// Workaround for weird HSE behaviors
// (Observed only on Rev.V H750, but may also apply to H743 and Rev.V.)
#define USE_H7_HSERDY_SLOW_WORKAROUND
#define USE_H7_HSE_TIMEOUT_WORKAROUND
#endif

// HSE clock configuration, originally taken from
// STM32Cube_FW_H7_V1.3.0/Projects/STM32H743ZI-Nucleo/Examples/RCC/RCC_ClockConfig/Src/main.c
static void SystemClockHSE_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

#ifdef notdef
    // CSI has been disabled at SystemInit().
    // HAL_RCC_ClockConfig() will fail because CSIRDY is off.

    /* -1- Select CSI as system clock source to allow modification of the PLL configuration */

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_CSI;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
        /* Initialization Error */
        ErrorHandler();
    }
#endif

    pllConfig_t *pll1Config;

#if defined(STM32H743xx) || defined(STM32H750xx)
    pll1Config = (HAL_GetREVID() == REV_ID_V) ? &pll1ConfigRevV : &pll1ConfigRevY;
#endif

    // Configure voltage scale.
    // It has been pre-configured at PWR_REGULATOR_VOLTAGE_SCALE1,
    // and it may stay or overridden by PWR_REGULATOR_VOLTAGE_SCALE0 depending on the clock config.

    __HAL_PWR_VOLTAGESCALING_CONFIG(pll1Config->vos);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
        // Empty
    }

    /* -2- Enable HSE  Oscillator, select it as PLL source and finally activate the PLL */

#ifdef USE_H7_HSERDY_SLOW_WORKAROUND
    // With reference to 2.3.22 in the ES0250 Errata for the L476.
    // Applying the same workaround here in the vain hopes that it improves startup times.
    // Randomly the HSERDY bit takes AGES, over 10 seconds, to be set.

    __HAL_RCC_GPIOH_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_RESET);

    GPIO_InitTypeDef  gpio_initstruct;
    gpio_initstruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio_initstruct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_initstruct.Pull = GPIO_NOPULL;
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(GPIOH, &gpio_initstruct);
#endif

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON; // Even Nucleo-H473ZI and Nucleo-H7A3ZI work without RCC_HSE_BYPASS

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = pll1Config->m;
    RCC_OscInitStruct.PLL.PLLN = pll1Config->n;
    RCC_OscInitStruct.PLL.PLLP = pll1Config->p;
    RCC_OscInitStruct.PLL.PLLQ = pll1Config->q;
    RCC_OscInitStruct.PLL.PLLR = pll1Config->r;

    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLRGE = pll1Config->vciRange;
    HAL_StatusTypeDef status = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    if (status != HAL_OK) {
        /* Initialization Error */
        ErrorHandler();
    }

    // Configure PLL2 and PLL3
    // Use of PLL2 and PLL3 are not determined yet.
    // A review of total system wide clock requirements is necessary.


    // Configure SCGU (System Clock Generation Unit)
    // Select PLL as system clock source and configure bus clock dividers.
    //
    // Clock type and divider member names do not have direct visual correspondence.
    // Here is how these correspond:
    //   RCC_CLOCKTYPE_SYSCLK           sys_ck
    //   RCC_CLOCKTYPE_HCLK             AHBx (rcc_hclk1,rcc_hclk2,rcc_hclk3,rcc_hclk4)
    //   RCC_CLOCKTYPE_D1PCLK1          APB3 (rcc_pclk3)
    //   RCC_CLOCKTYPE_PCLK1            APB1 (rcc_pclk1)
    //   RCC_CLOCKTYPE_PCLK2            APB2 (rcc_pclk2)
    //   RCC_CLOCKTYPE_D3PCLK1          APB4 (rcc_pclk4)

    RCC_ClkInitStruct.ClockType = ( \
        RCC_CLOCKTYPE_SYSCLK | \
        RCC_CLOCKTYPE_HCLK | \
        RCC_CLOCKTYPE_D1PCLK1 | \
        RCC_CLOCKTYPE_PCLK1 | \
        RCC_CLOCKTYPE_PCLK2  | \
        RCC_CLOCKTYPE_D3PCLK1);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.AHBCLKDivider = MCU_HCLK_DIVIDER;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, MCU_FLASH_LATENCY) != HAL_OK) {
        /* Initialization Error */
        ErrorHandler();
    }

    /* -4- Optional: Disable CSI Oscillator (if the HSI is no more needed by the application)*/
    RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_CSI;
    RCC_OscInitStruct.CSIState        = RCC_CSI_OFF;
    RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        /* Initialization Error */
        ErrorHandler();
    }
}

void SystemClock_Config(void)
{
    // Configure power supply

    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    // Pre-configure voltage scale to PWR_REGULATOR_VOLTAGE_SCALE1.
    // SystemClockHSE_Config may configure PWR_REGULATOR_VOLTAGE_SCALE0.

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
        // Empty
    }

    SystemClockHSE_Config();

    /*activate CSI clock mondatory for I/O Compensation Cell*/

    __HAL_RCC_CSI_ENABLE() ;

    /* Enable SYSCFG clock mondatory for I/O Compensation Cell */

    __HAL_RCC_SYSCFG_CLK_ENABLE() ;

    /* Enables the I/O Compensation Cell */

    HAL_EnableCompensationCell();

    HAL_Delay(10);
}


void SystemInit (void)
{
    // Reset the RCC clock configuration to the default reset state
    // Set HSION bit
    RCC->CR = RCC_CR_HSION;

    // Reset CFGR register
    RCC->CFGR = 0x00000000;

    // Reset HSEON, CSSON , CSION,RC48ON, CSIKERON PLL1ON, PLL2ON and PLL3ON bits

    // XXX Don't do this until we are established with clock handling
    // RCC->CR &= (uint32_t)0xEAF6ED7F;

    // Instead, we explicitly turn those on
    RCC->CR |= RCC_CR_CSION;
    RCC->CR |= RCC_CR_HSION;
    RCC->CR |= RCC_CR_HSEON;
    RCC->CR |= RCC_CR_HSI48ON;

    /* Reset D1CFGR register */
    RCC->D1CFGR = 0x00000000;

    /* Reset D2CFGR register */
    RCC->D2CFGR = 0x00000000;

    /* Reset D3CFGR register */
    RCC->D3CFGR = 0x00000000;

    /* Reset PLLCKSELR register */
    RCC->PLLCKSELR = 0x00000000;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x00000000;
    /* Reset PLL1DIVR register */
    RCC->PLL1DIVR = 0x00000000;
    /* Reset PLL1FRACR register */
    RCC->PLL1FRACR = 0x00000000;

    /* Reset PLL2DIVR register */
    RCC->PLL2DIVR = 0x00000000;

    /* Reset PLL2FRACR register */

    RCC->PLL2FRACR = 0x00000000;
    /* Reset PLL3DIVR register */
    RCC->PLL3DIVR = 0x00000000;

    /* Reset PLL3FRACR register */
    RCC->PLL3FRACR = 0x00000000;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Disable all interrupts */
    RCC->CIER = 0x00000000;

    /* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
  *((__IO uint32_t*)0x51008108) = 0x00000001;

    SCB->VTOR = FLASH_BANK1_BASE | VECT_TAB_OFFSET;       /* Vector Table Relocation in Internal FLASH */

#ifdef USE_HAL_DRIVER
    HAL_Init();
#endif

    SystemClock_Config();
    SystemCoreClockUpdate();

    // Enable CPU L1-Cache
    SCB_EnableICache();
    SCB_EnableDCache();
}

/**
  * @brief  Update SystemCoreClock variable according to Clock Register Values.
  *         The SystemCoreClock variable contains the core clock , it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *
  * @note   Each time the core clock changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.
  *
  * @note   - The system frequency computed by this function is not the real
  *           frequency in the chip. It is calculated based on the predefined
  *           constant and the selected clock source:
  *
  *           - If SYSCLK source is CSI, SystemCoreClock will contain the CSI_VALUE(*)
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(**)
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(***)
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the CSI_VALUE(*),
  *             HSI_VALUE(**) or HSE_VALUE(***) multiplied/divided by the PLL factors.
  *
  *         (*) CSI_VALUE is a constant defined in stm32h7xx_hal.h file (default value
  *             4 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.
  *         (**) HSI_VALUE is a constant defined in stm32h7xx_hal.h file (default value
  *             64 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.
  *
  *         (***)HSE_VALUE is a constant defined in stm32h7xx_hal.h file (default value
  *              25 MHz), user has to ensure that HSE_VALUE is same as the real
  *              frequency of the crystal used. Otherwise, this function may
  *              have wrong result.
  *
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.
  * @param  None
  * @retval None
  */
void SystemCoreClockUpdate (void)
{
    SystemCoreClock = HAL_RCC_GetSysClockFreq();
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
