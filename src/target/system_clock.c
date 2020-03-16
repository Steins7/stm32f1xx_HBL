/* mbed Microcontroller Library
* Copyright (c) 2006-2017 ARM Limited
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/**
  * This file configures the system clock as follows:
  *-----------------------------------------------------------------------------
  * System clock source                | 1- PLL_HSE_EXTC        | 3- PLL_HSI
  *                                    | (external 8 MHz clock) | (internal 8 MHz)
  *                                    | 2- PLL_HSE_XTAL        |
  *                                    | (external 8 MHz xtal)  |
  *-----------------------------------------------------------------------------
  * SYSCLK(MHz)                        | 72                     | 64
  *-----------------------------------------------------------------------------
  * AHBCLK (MHz)                       | 72                     | 64
  *-----------------------------------------------------------------------------
  * APB1CLK (MHz)                      | 36                     | 32
  *-----------------------------------------------------------------------------
  * APB2CLK (MHz)                      | 72                     | 64
  *-----------------------------------------------------------------------------
  * USB capable (48 MHz precise clock) | NO                     | NO
  *-----------------------------------------------------------------------------
  ******************************************************************************
  */

#include "stm32f1xx.h"

/*!< Uncomment the following line if you need to relocate your vector Table in
     Internal SRAM. */
/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x00000000U /*!< Vector Table base offset field. 
                                  This value must be a multiple of 0x200. */

/**
  * @brief  Setup the microcontroller system
  *         Initialize the Embedded Flash Interface, the PLL and update the
  *         SystemCoreClock variable.
  * @note   This function should be used only after reset.
  * @param  None
  * @retval None
  */
void SystemInit (void)
{
    /* Reset the RCC clock configuration to the default reset state(for debug purpose) */
    /* Set HSION bit */
    RCC->CR |= 0x00000001U;

    /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
#if !defined(STM32F105xC) && !defined(STM32F107xC)
    RCC->CFGR &= 0xF8FF0000U;
#else
    RCC->CFGR &= 0xF0FF0000U;
#endif /* STM32F105xC */

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= 0xFEF6FFFFU;

    /* Reset HSEBYP bit */
    RCC->CR &= 0xFFFBFFFFU;

    /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
    RCC->CFGR &= 0xFF80FFFFU;

#if defined(STM32F105xC) || defined(STM32F107xC)
    /* Reset PLL2ON and PLL3ON bits */
    RCC->CR &= 0xEBFFFFFFU;

    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x00FF0000U;

    /* Reset CFGR2 register */
    RCC->CFGR2 = 0x00000000U;
#elif defined(STM32F100xB) || defined(STM32F100xE)
    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x009F0000U;

    /* Reset CFGR2 register */
    RCC->CFGR2 = 0x00000000U;
#else
    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x009F0000U;
#endif /* STM32F105xC */

#if defined(STM32F100xE) || defined(STM32F101xE) || defined(STM32F101xG) || defined(STM32F103xE) || defined(STM32F103xG)
#ifdef DATA_IN_ExtSRAM
    SystemInit_ExtMemCtl();
#endif /* DATA_IN_ExtSRAM */
#endif

#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif

}

