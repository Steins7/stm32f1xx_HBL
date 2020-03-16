/*
==============================================================================
##### RCC specific features #####
==============================================================================
[..]  
After reset the device is running from Internal High Speed oscillator 
(HSI 16MHz) with Flash 0 wait state, Flash prefetch buffer, D-Cache 
and I-Cache are disabled, and all peripherals are off except internal
SRAM, Flash and JTAG.
(+) There is no prescaler on High speed (AHB) and Low speed (APB) busses;
all peripherals mapped on these busses are running at HSI speed.
(+) The clock for all peripherals is switched off, except the SRAM and FLASH.
(+) All GPIOs are in input floating state, except the JTAG pins which
are assigned to be used for debug purpose.

[..]          
Once the device started from reset, the user application has to:        
(+) Configure the clock source to be used to drive the System clock
(if the application needs higher frequency/performance)
(+) Configure the System clock frequency and Flash settings  
(+) Configure the AHB and APB busses prescalers
(+) Enable the clock for the peripheral(s) to be used
(+) Configure the clock source(s) for peripherals which clocks are not
derived from the System clock (I2S, RTC, ADC, USB OTG FS/SDIO/RNG)

##### RCC Limitations #####
==============================================================================
[..]  
A delay between an RCC peripheral clock enable and the effective peripheral 
enabling should be taken into account in order to manage the peripheral 
read/write from/to registers.
(+) This delay depends on the peripheral mapping.
(+) If peripheral is mapped on AHB: the delay is 2 AHB clock cycle 
after the clock enable bit is set on the hardware register
(+) If peripheral is mapped on APB: the delay is 2 APB clock cycle 
after the clock enable bit is set on the hardware register

[..]  
Possible Workarounds:
(#) Enable the peripheral clock sometimes before the peripheral read/write 
register is required.
(#) For AHB peripheral, insert two dummy read to the peripheral register.
(#) For APB peripheral, insert a dummy read to the peripheral register.
*/

#include "rcc.h"

/* HPRE: AHB high-speed prescaler */
#define RCC_CFGR_HPRE_DIV_NONE			0x0
#define RCC_CFGR_HPRE_DIV_2				(0x8 + 0)
#define RCC_CFGR_HPRE_DIV_4				(0x8 + 1)
#define RCC_CFGR_HPRE_DIV_8				(0x8 + 2)
#define RCC_CFGR_HPRE_DIV_16			(0x8 + 3)
#define RCC_CFGR_HPRE_DIV_64			(0x8 + 4)
#define RCC_CFGR_HPRE_DIV_128			(0x8 + 5)
#define RCC_CFGR_HPRE_DIV_256			(0x8 + 6)
#define RCC_CFGR_HPRE_DIV_512			(0x8 + 7)

/* PPRE1/2: APB high-speed prescalers */
#define RCC_CFGR_PPRE_DIV_NONE			0x0
#define RCC_CFGR_PPRE_DIV_2				0x4
#define RCC_CFGR_PPRE_DIV_4				0x5
#define RCC_CFGR_PPRE_DIV_8				0x6
#define RCC_CFGR_PPRE_DIV_16			0x7

/* PPLMUL: PPL multiplier */
#define RCC_CFGR_PLLMUL(fac)			(fac + 2)

/* ADC: ADCPRE prescalers */
#define RCC_CFGR_ADCPRE_DIV_2			0x0
#define RCC_CFGR_ADCPRE_DIV_4			0x1
#define RCC_CFGR_ADCPRE_DIV_6			0x2
#define RCC_CFGR_ADCPRE_DIV_8			0x3

enum rcc_osc {
	RCC_HSI,
	RCC_HSE,
	RCC_PLL,
	RCC_LSI,
	RCC_LSE
};

struct ClockConfig_t {
	uint8_t		type;
	uint8_t		pll_src;
	uint8_t		pllmul;
	uint8_t		hpre;
	uint8_t		ppre1;
	uint8_t		ppre2;
	uint8_t		adcpre;
	uint32_t	flash_cfg;
	uint32_t	ahb_freq;
	uint32_t	apb1_freq;
	uint32_t	apb2_freq;
};

static struct ClockConfig_t _clock_config[] = {
	{/* Performance Mode */
		.type = RCC_PLL,
		.pll_src = RCC_HSE, //8MHz
		.pllmul = RCC_CFGR_PLLMUL(9), 	//freq should noot exceed 72MHz
		.hpre = RCC_CFGR_HPRE_DIV_NONE,
		.ppre1 = RCC_CFGR_PPRE_DIV_2, 	//freq should not exceed 36MHz
		.ppre2 = RCC_CFGR_PPRE_DIV_NONE,
		.adcpre = RCC_CFGR_ADCPRE_DIV_6,	//freq should not exceed 14MHz
		.flash_cfg = FLASH_ACR_LATENCY_2,
		.ahb_freq = 72000000,
		.apb1_freq = 36000000,
		.apb2_freq = 72000000
	},

	{/* Powersave Mode */
		.type = RCC_HSE,
		.hpre = RCC_CFGR_HPRE_DIV_16,
		.ppre1 = RCC_CFGR_PPRE_DIV_16,
		.ppre2 = RCC_CFGR_PPRE_DIV_16,
		.adcpre = RCC_CFGR_ADCPRE_DIV_2,
		.flash_cfg = FLASH_ACR_LATENCY_0,
		.ahb_freq = 500000,
		.apb1_freq = 500000,
		.apb2_freq = 500000
	}
};

static void rcc_osc_on(enum rcc_osc osc)
{
	switch (osc) {
		case RCC_HSI:
			if (!(RCC->CR & RCC_CR_HSION)) {
				RCC->CR |= RCC_CR_HSION;
				while ((RCC->CR & RCC_CR_HSIRDY)==0);
			}
			break;
		case RCC_HSE:
			if (!(RCC->CR & RCC_CR_HSEON)) {
				RCC->CR |= RCC_CR_HSEON;
				while ((RCC->CR & RCC_CR_HSERDY)==0);
			}
			break;
		case RCC_PLL:
			if (!(RCC->CR & RCC_CR_PLLON)) {
				RCC->CR |= RCC_CR_PLLON;
				while ((RCC->CR & RCC_CR_PLLRDY)==0);
			}
			break;
		case RCC_LSI:
			if (!(RCC->CSR & RCC_CSR_LSION)) {
				RCC->CSR |= RCC_CSR_LSION;
				while ((RCC->CSR & RCC_CSR_LSIRDY)==0);
			}
			break;
		case RCC_LSE:
			if (!(RCC->BDCR & RCC_BDCR_LSEON)) {
				RCC->BDCR |= RCC_BDCR_LSEON;
				while ((RCC->BDCR & RCC_BDCR_LSERDY)==0);
			}
			break;
	}
}

static void rcc_osc_off(enum rcc_osc osc)
{
	switch (osc) {
		case RCC_HSI:
			RCC->CR &= ~RCC_CR_HSION;
			break;
		case RCC_HSE:
			RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_HSEBYP | RCC_CR_CSSON);
			break;
		case RCC_PLL:
			RCC->CR &= ~RCC_CR_PLLON;
			break;
		case RCC_LSI:
			RCC->CSR &= ~RCC_CSR_LSION;
			break;
		case RCC_LSE:
			RCC->BDCR &= ~RCC_BDCR_LSEON;
			break;
	}
}

static void rcc_set_sysclk(enum rcc_osc osc)
{
	RCC->CFGR = (RCC->CFGR & ~0x3) | (osc & 3);
	while (((RCC->CFGR & 0xC)>>2) != osc);
}

void rcc_config_clock(uint32_t config, Clock_t *sysclks)
{
	struct ClockConfig_t *clk;

	if (config < CLOCK_CONFIG_END) {
		clk=&(_clock_config[config]);
	} else {
		clk=&(_clock_config[CLOCK_CONFIG_PERFORMANCE]);
	}

	if (clk->type == RCC_HSE) {			// HSE Clock
		rcc_osc_on(RCC_HSE);
		rcc_set_sysclk(RCC_HSE);
		rcc_osc_off(RCC_PLL);
		rcc_osc_off(RCC_HSI);

	} else if (clk->type == RCC_PLL) {
		// enable PWR module clocking
		RCC->APB1ENR |= 1<<28;

		if (clk->pll_src == RCC_HSE) {	// HSE Clock src
			rcc_osc_on(RCC_HSE);
		} else {						// Default: HSI Clock src
			rcc_osc_on(RCC_HSI);
		}

		// configure prescalers for 
		//  AHB: AHBCLK > 25MHz
		//  APB1: APB1CLK <= 36MHz
		//  APB2: APB2CLK <= 72MHz
		RCC->CFGR = ( RCC->CFGR & ~((0x3F<<8) | (0xF<<4) | (0x3<<14)) ) |
			((clk->hpre & 0xF) << 4) |
			((clk->ppre1 & 0x7) << 8) |
			((clk->ppre2 & 0x7) << 11)|
			(clk->adcpre << 14);

		// configure PLL
		RCC->CFGR &= !(0xF<<18);
		RCC->CFGR |= clk->pllmul<<18;

		// enable PLL oscillator
		rcc_osc_on(RCC_PLL);

		// set Flash timings
		FLASH->ACR &= !0x8;
		FLASH->ACR |= clk->flash_cfg;
		//TODO set buffer bits

		// connect to PLL
		rcc_set_sysclk(RCC_PLL);

		// stop unused clock
		if ((clk->pll_src == RCC_HSE) && (RCC->CR & RCC_CR_HSION))
			rcc_osc_off(RCC_HSI);
		else
			rcc_osc_off(RCC_HSE);

	} else {							// Default: HSI Clock
		rcc_osc_on(RCC_HSI);
		rcc_set_sysclk(RCC_HSI);
		rcc_osc_off(RCC_PLL);
		rcc_osc_off(RCC_HSE);
	}
	sysclks->ahb_freq = clk->ahb_freq;
	sysclks->apb1_freq = clk->apb1_freq;
	sysclks->apb2_freq = clk->apb2_freq;
	sysclks->apb1_timer_freq = 
		clk->ppre1==RCC_CFGR_PPRE_DIV_NONE ? clk->apb1_freq : 2*clk->apb1_freq;
	sysclks->apb2_timer_freq = 
		clk->ppre2==RCC_CFGR_PPRE_DIV_NONE ? clk->apb2_freq : 2*clk->apb2_freq;
}
