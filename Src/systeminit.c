/*
 * systeminit.c
 *
 *  Created on: Jun 13, 2026
 *      Author: nhduong
 */
#include <stdint.h>
#include "system_stm32f4xx.h"
#include "stm32f4xx.h"
#include "Helpers/logger.h"

LogLevel system_log_level = LOG_LEVEL_DEBUG;

/**
 * @brief Configures system clock to 72 MHz using HSE and PLL.
 *
 * Clock tree:
 * - Source:   HSE (8 MHz external crystal)
 * - PLL:      M=4, N=72, P=2, Q=3
 * - SYSCLK:   72 MHz
 * - AHB:      72 MHz (prescaler = 1)
 * - APB1:     36 MHz (prescaler = 2)
 * - APB2:     72 MHz (prescaler = 1)
 * - USB:      48 MHz (PLLQ = 3)
 *
 * @note Flash latency is set to 2WS before increasing clock speed.
 * @note HSI is disabled after switching to PLL to save power.
 */
static void Configure_Clock(void)
{
	/* Configures flash latency */
	MODIFY_REG(FLASH->ACR,
		FLASH_ACR_LATENCY,
		_VAL2FLD(FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_2WS) //FLASH_ACR_LATENCY_2WS << FLASH_ACR_LATENCY_Pos
	);

	/* Enables HSE */
	SET_BIT(RCC->CR, RCC_CR_HSEON);

	/* Waits until HSE is stable */
	while (!READ_BIT(RCC->CR, RCC_CR_HSERDY));

	/* Configures PLL: source = HSE, PLLCLK = 72MHz */
	MODIFY_REG(RCC->PLLCFGR,
		RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLQ | RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLP,
		_VAL2FLD(RCC_PLLCFGR_PLLM, 4) | _VAL2FLD(RCC_PLLCFGR_PLLN, 72) | _VAL2FLD(RCC_PLLCFGR_PLLQ, 3) | RCC_PLLCFGR_PLLSRC_HSE
	);

	/* Enables PLL module */
	SET_BIT(RCC->CR, RCC_CR_PLLON);

	/* Waits until PLL is stable */
	while (!READ_BIT(RCC->CR, RCC_CR_PLLRDY));

	/* Switches system clock to PLL */
	MODIFY_REG(RCC->CFGR,
		RCC_CFGR_SW,
		_VAL2FLD(RCC_CFGR_SW, RCC_CFGR_SW_PLL)
	);

	/* Configures PPRE1 = 2, (PPRE2 = 1, HPRE = 1 by default) */
	MODIFY_REG(RCC->CFGR,
		RCC_CFGR_PPRE1,
		_VAL2FLD(RCC_CFGR_PPRE1, 4)
	);

	/* Waits until PLL is used */
	while(READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

	/* Disables HSI */
	CLEAR_BIT(RCC->CR, RCC_CR_HSION);
}

/**
 * @brief System initialization entry point called before main().
 *
 * Initializes system clock. This function is declared as a weak symbol
 * in the CMSIS startup file and overridden here.
 */
void SystemInit(void)
{
    /* Init Clock */
    Configure_Clock();
}
