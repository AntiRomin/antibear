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
#define VECT_TAB_OFFSET  0x00000000UL /*!< Vector Table base offset field.
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

/** @addtogroup STM32H7xx_System_Private_FunctionPrototypes
  * @{
  */
static void SystemInit_ExtMemCtl(void);

/**
  * @}
  */

/** @addtogroup STM32H7xx_System_Private_Functions
  * @{
  */

/**
  * @brief  Setup the microcontroller system
  *         Initialize the FPU setting, vector table location.
  * @param  None
  * @retval None
  */

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 480000000 (CPU Clock)
  *            HCLK(Hz)                       = 240000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  240MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  240MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  240MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  240MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 192
  *            PLL_P                          = 2
  *            PLL_Q                          = 20
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClockHSE_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    HAL_StatusTypeDef status = HAL_OK;

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
    RCC_OscInitStruct.CSIState = RCC_CSI_OFF;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 20;
    RCC_OscInitStruct.PLL.PLLR = 2;

    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    status = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if (status != HAL_OK) {
        // TODO: Handle error
        while(1);
    }

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
        RCC_CLOCKTYPE_PCLK2 | \
        RCC_CLOCKTYPE_D3PCLK1);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    status = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
    if (status != HAL_OK) {
        // TODO: Handle error
        while(1);
    }
}

static void SystemClock_Config(void)
{
    // Supply configuration update enable
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    // Configure the main internal regulator output voltage
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    SystemClockHSE_Config();

    // Enable SYSCFG clock mondatory for I/O Compensation Cell
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    // Configure peripheral clocks

    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

    // Configure UART peripheral clock sources
    //
    // Possible sources:
    //   D2PCLK1 (pclk1 for APB1 = USART234578)
    //   D2PCLK2 (pclk2 for APB2 = USART16)
    //   PLL2 (pll2_q_ck)
    //   PLL3 (pll3_q_ck)
    //   HSI (hsi_ck)
    //   CSI (csi_ck)
    //   LSE (lse_ck)

    RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART16 | RCC_PERIPHCLK_USART234578;
    RCC_PeriphClkInit.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    RCC_PeriphClkInit.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

    // Configure QSPI peripheral clock sources
    //
    // Possible sources for QSPI:
    //   D1HCLK (hclk for AHB1)
    //   PLL1 (pll1_q_ck)
    //   PLL2 (pll2_r_ck)
    //   CLKP (per_ck)

    RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_QSPI;
    RCC_PeriphClkInit.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
    HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

    // Configure USB OTG peripheral clock sources
    //
    // Possible sources for USB OTG:
    //   PLL1 (pll1_q_ck)
    //   PLL3 (pll3_q_ck)
    //   HSI48 (hsi48_ck)

    RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    RCC_PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
    HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);
}

void SystemInit (void)
{
    memProtReset();

    // FPU settings
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << (10*2))|(3UL << (11*2)));  // Set CP10 and CP11 Full Access
#endif

    // Reset the RCC clock configuration to the default reset state
    // Set HSION bit
    RCC->CR |= RCC_CR_HSION;

    // Reset CFGR register
    RCC->CFGR = 0x00000000;

    // Reset HSEON, CSSON , CSION,RC48ON, CSIKERON PLL1ON, PLL2ON and PLL3ON bits
    RCC->CR &= 0xEAF6ED7FU;

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
    RCC->CR &= 0xFFFBFFFFU;

    /* Disable all interrupts */
    RCC->CIER = 0x00000000;

    /* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
    if((DBGMCU->IDCODE & 0xFFFF0000U) < 0x20000000U)
    {
        /* if stm32h7 revY*/
        /* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
        *((__IO uint32_t*)0x51008108) = 0x000000001U;
    }

    /*
     * Disable the FMC bank1 (enabled after reset).
     * This, prevents CPU speculation access on this bank which blocks the use of FMC during
     * 24us. During this time the others FMC master (such as LTDC) cannot use it!
     */
    FMC_Bank1_R->BTCR[0] = 0x000030D2;

    /* Configure the Vector Table location add offset address ------------------*/
#ifdef VECT_TAB_SRAM
    SCB->VTOR = D1_AXISRAM_BASE  | VECT_TAB_OFFSET;       /* Vector Table Relocation in Internal SRAM */
#else
    SCB->VTOR = FLASH_BANK1_BASE | VECT_TAB_OFFSET;       /* Vector Table Relocation in Internal FLASH */
#endif

    HAL_Init();

    SystemClock_Config();
    SystemCoreClockUpdate();

    SystemInit_ExtMemCtl();

    initialiseMemorySections();

    // Configure the MPU attributes
    memProtConfigure();

    // Enable the CPU Cache
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
    uint32_t pllp, pllsource, pllm, pllfracen, hsivalue, tmp;
    float_t fracn1, pllvco;

    /* Get SYSCLK source -------------------------------------------------------*/

    switch (RCC->CFGR & RCC_CFGR_SWS)
    {
       case RCC_CFGR_SWS_HSI:  /* HSI used as system clock source */
            SystemCoreClock = (uint32_t) (HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV)>> 3));

            break;

        case RCC_CFGR_SWS_CSI:  /* CSI used as system clock  source */
            SystemCoreClock = CSI_VALUE;
            break;

        case RCC_CFGR_SWS_HSE:  /* HSE used as system clock  source */
            SystemCoreClock = HSE_VALUE;
            break;

        case RCC_CFGR_SWS_PLL1:  /* PLL1 used as system clock  source */

            /* PLL_VCO = (HSE_VALUE or HSI_VALUE or CSI_VALUE/ PLLM) * PLLN
            SYSCLK = PLL_VCO / PLLR
            */
            pllsource = (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC);
            pllm = ((RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM1)>> 4)  ;
            pllfracen = ((RCC->PLLCFGR & RCC_PLLCFGR_PLL1FRACEN)>>RCC_PLLCFGR_PLL1FRACEN_Pos);
            fracn1 = (float_t)(uint32_t)(pllfracen* ((RCC->PLL1FRACR & RCC_PLL1FRACR_FRACN1)>> 3));

            if (pllm != 0U)
            {
                switch (pllsource)
                {
                    case RCC_PLLCKSELR_PLLSRC_HSI:  /* HSI used as PLL clock source */

                        hsivalue = (HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV)>> 3)) ;
                        pllvco = ( (float_t)hsivalue / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1/(float_t)0x2000) +(float_t)1 );

                        break;

                    case RCC_PLLCKSELR_PLLSRC_CSI:  /* CSI used as PLL clock source */
                        pllvco = ((float_t)CSI_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1/(float_t)0x2000) +(float_t)1 );
                        break;

                    case RCC_PLLCKSELR_PLLSRC_HSE:  /* HSE used as PLL clock source */
                        pllvco = ((float_t)HSE_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1/(float_t)0x2000) +(float_t)1 );
                        break;

                    default:
                        pllvco = ((float_t)CSI_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1/(float_t)0x2000) +(float_t)1 );
                        break;
                }
                pllp = (((RCC->PLL1DIVR & RCC_PLL1DIVR_P1) >>9) + 1U ) ;
                SystemCoreClock =  (uint32_t)(float_t)(pllvco/(float_t)pllp);
            }
            else
            {
                SystemCoreClock = 0U;
            }
            break;

        default:
            SystemCoreClock = CSI_VALUE;
            break;
    }

    /* Compute SystemClock frequency --------------------------------------------------*/

    tmp = D1CorePrescTable[(RCC->D1CFGR & RCC_D1CFGR_D1CPRE)>> RCC_D1CFGR_D1CPRE_Pos];
    /* SystemCoreClock frequency : CM7 CPU frequency  */
    SystemCoreClock >>= tmp;

    /* SystemD2Clock frequency : AXI and AHBs Clock frequency  */
    SystemD2Clock = (SystemCoreClock >> ((D1CorePrescTable[(RCC->D1CFGR & RCC_D1CFGR_HPRE)>> RCC_D1CFGR_HPRE_Pos]) & 0x1FU));
}

/**
  * @brief  Setup the external memory controller.
  *         Called in startup_stm32h7xx.s before jump to main.
  *         This function configures the external memories (SRAM/SDRAM)
  *         This SRAM/SDRAM will be used as program data memory (including heap and stack).
  * @param  None
  * @retval None
  */
void SystemInit_ExtMemCtl(void)
{
    register uint32_t tmpreg = 0, timeout = 0xFFFF;
    register __IO uint32_t index;

    /* Enable GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH and GPIOI interface clock */
    RCC->AHB4ENR  |= 0x000001FC;
    /* Connect PCx pins to FMC Alternate function */
    GPIOC->AFR[0]  = 0x0000000C;
    GPIOC->AFR[1]  = 0x00000000;
    /* Configure PCx pins in Alternate function mode */
    GPIOC->MODER   = 0xFFFFFFFE;
    /* Configure PCx pins speed to 50 MHz */
    GPIOC->OSPEEDR = 0x00000003;
    /* Configure PCx pins Output type to push-pull */
    GPIOC->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PCx pins */
    GPIOC->PUPDR   = 0x00000001;
    /* Connect PDx pins to FMC Alternate function */
    GPIOD->AFR[0]  = 0x000000CC;
    GPIOD->AFR[1]  = 0xCC000CCC;
    /* Configure PDx pins in Alternate function mode */
    GPIOD->MODER   = 0xAFEAFFFA;
    /* Configure PDx pins speed to 50 MHz */
    GPIOD->OSPEEDR = 0xF03F000F;
    /* Configure PDx pins Output type to push-pull */
    GPIOD->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PDx pins */
    GPIOD->PUPDR   = 0x50150005;
    /* Connect PEx pins to FMC Alternate function */
    GPIOE->AFR[0]  = 0xC00000CC;
    GPIOE->AFR[1]  = 0xCCCCCCCC;
    /* Configure PEx pins in Alternate function mode */
    GPIOE->MODER   = 0xAAAABFFA;
    /* Configure PEx pins speed to 50 MHz */
    GPIOE->OSPEEDR = 0xFFFFC00F;
    /* Configure PEx pins Output type to push-pull */
    GPIOE->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PEx pins */
    GPIOE->PUPDR   = 0x55554005;
    /* Connect PFx pins to FMC Alternate function */
    GPIOF->AFR[0]  = 0x00CCCCCC;
    GPIOF->AFR[1]  = 0xCCCCC000;
    /* Configure PFx pins in Alternate function mode */
    GPIOF->MODER   = 0xAABFFAAA;
    /* Configure PFx pins speed to 50 MHz */
    GPIOF->OSPEEDR = 0xFFC00FFF;
    /* Configure PFx pins Output type to push-pull */
    GPIOF->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PFx pins */
    GPIOF->PUPDR   = 0x55400555;
    /* Connect PGx pins to FMC Alternate function */
    GPIOG->AFR[0]  = 0x00CC0CCC;
    GPIOG->AFR[1]  = 0xC000000C;
    /* Configure PGx pins in Alternate function mode */
    GPIOG->MODER   = 0xBFFEFAEA;
    /* Configure PGx pins speed to 50 MHz */
    GPIOG->OSPEEDR = 0xC0030F3F;
    /* Configure PGx pins Output type to push-pull */
    GPIOG->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PGx pins */
    GPIOG->PUPDR   = 0x40010515;
    /* Connect PHx pins to FMC Alternate function */
    GPIOH->AFR[0]  = 0xCC000000;
    GPIOH->AFR[1]  = 0xCCCCCCCC;
    /* Configure PHx pins in Alternate function mode */
    GPIOH->MODER   = 0xAAAAAFFF;
    /* Configure PHx pins speed to 50 MHz */
    GPIOH->OSPEEDR = 0xFFFFF000;
    /* Configure PHx pins Output type to push-pull */
    GPIOH->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PHx pins */
    GPIOH->PUPDR   = 0x55555000;
    /* Connect PIx pins to FMC Alternate function */
    GPIOI->AFR[0]  = 0xCCCCCCCC;
    GPIOI->AFR[1]  = 0x00000CC0;
    /* Configure PIx pins in Alternate function mode */
    GPIOI->MODER   = 0xFFEBAAAA;
    /* Configure PIx pins speed to 50 MHz */
    GPIOI->OSPEEDR = 0x003CFFFF;
    /* Configure PIx pins Output type to push-pull */
    GPIOI->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PIx pins */
    GPIOI->PUPDR   = 0x00145555;
/*-- FMC Configuration ------------------------------------------------------*/
    /* Enable the FMC interface clock */
    (RCC->AHB3ENR |= (RCC_AHB3ENR_FMCEN));
    /*SDRAM Timing and access interface configuration*/
    /*LoadToActiveDelay    = 2
      ExitSelfRefreshDelay = 10
      SelfRefreshTime      = 6
      RowCycleDelay        = 8
      WriteRecoveryTime    = 2
      RPDelay              = 2
      RCDDelay             = 2
      SDBank             = FMC_SDRAM_BANK2
      ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9
      RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13
      MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32
      InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4
      CASLatency         = FMC_SDRAM_CAS_LATENCY_2
      WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE
      SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2
      ReadBurst          = FMC_SDRAM_RBURST_ENABLE
      ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1*/

    FMC_Bank5_6_R->SDCR[0] = 0x00003800;
    FMC_Bank5_6_R->SDCR[1] = 0x00000169;
    FMC_Bank5_6_R->SDTR[0] = 0x00107000;
    FMC_Bank5_6_R->SDTR[1] = 0x01010591;

    /* SDRAM initialization sequence */
    /* Clock enable command */
    FMC_Bank5_6_R->SDCMR = 0x00000009;
    tmpreg = FMC_Bank5_6_R->SDSR & 0x00000020;
    while((tmpreg != 0) && (timeout-- > 0))
    {
      tmpreg = FMC_Bank5_6_R->SDSR & 0x00000020;
    }

    /* Delay */
    for (index = 0; index<1000; index++);

    /* PALL command */
    FMC_Bank5_6_R->SDCMR = 0x0000000A;
    timeout = 0xFFFF;
    while((tmpreg != 0) && (timeout-- > 0))
    {
      tmpreg = FMC_Bank5_6_R->SDSR & 0x00000020;
    }

    FMC_Bank5_6_R->SDCMR = 0x000000EB;
    timeout = 0xFFFF;
    while((tmpreg != 0) && (timeout-- > 0))
    {
      tmpreg = FMC_Bank5_6_R->SDSR & 0x00000020;
    }

    FMC_Bank5_6_R->SDCMR = 0x0004400C;
    timeout = 0xFFFF;
    while((tmpreg != 0) && (timeout-- > 0))
    {
      tmpreg = FMC_Bank5_6_R->SDSR & 0x00000020;
    }
    /* Set refresh count */
    tmpreg = FMC_Bank5_6_R->SDRTR;
    FMC_Bank5_6_R->SDRTR = (tmpreg | (0x00000395<<1));    // 64ms/8K * 120Mhz
    // FMC_Bank5_6_R->SDRTR = (tmpreg | (0x000003FB<<1));    // 64ms/8K * 133Mhz

    /* Disable write protection */
    tmpreg = FMC_Bank5_6_R->SDCR[1];
    FMC_Bank5_6_R->SDCR[1] = (tmpreg & 0xFFFFFDFF);

    /* SDRAM Bank2 remapped on FMC bank2 and still accessible at default mapping */
    FMC_Bank1_R->BTCR[0] |= 0x02000000;

    /*FMC controller Enable*/
    FMC_Bank1_R->BTCR[0] |= 0x80000000;
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
