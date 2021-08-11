//driver header
#include "io.h"

static OnIO io_cb[16]={
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

void EXTI0_IRQHandler() {
	if (io_cb[0]) (io_cb[0])();
	EXTI->PR = 1<<0;
}

void EXTI1_IRQHandler() {
	if (io_cb[1]) (io_cb[1])();
	EXTI->PR = 1<<1;
}

void EXTI2_IRQHandler() {
	if (io_cb[2]) (io_cb[2])();
	EXTI->PR = 1<<2;
}

void EXTI3_IRQHandler() {
	if (io_cb[3]) (io_cb[3])();
	EXTI->PR = 1<<3;
}

void EXTI4_IRQHandler() {
	if (io_cb[4]) (io_cb[4])();
	EXTI->PR = 1<<4;
}

void EXTI9_5_IRQHandler() {
	if (EXTI->PR & (1<<5)) {
		if (io_cb[5]) (io_cb[5])();
		EXTI->PR = 1<<5;
	} else if (EXTI->PR & (1<<6)) {
		if (io_cb[6]) (io_cb[6])();
		EXTI->PR = 1<<6;
	} else if (EXTI->PR & (1<<7)) {
		if (io_cb[7]) (io_cb[7])();
		EXTI->PR = 1<<7;
	} else if (EXTI->PR & (1<<8)) {
		if (io_cb[8]) (io_cb[8])();
		EXTI->PR = 1<<8;
	} else if (EXTI->PR & (1<<9)) {
		if (io_cb[9]) (io_cb[9])();
		EXTI->PR = 1<<9;
	}
}

void EXTI15_10_IRQHandler() {
	if (EXTI->PR & (1<<10)) {
		if (io_cb[10]) (io_cb[10])();
		EXTI->PR = 1<<10;
	} else if (EXTI->PR & (1<<11)) {
		if (io_cb[11]) (io_cb[11])();
		EXTI->PR = 1<<11;
	} else if (EXTI->PR & (1<<12)) {
		if (io_cb[12]) (io_cb[12])();
		EXTI->PR = 1<<12;
	} else if (EXTI->PR & (1<<13)) {
		if (io_cb[13]) (io_cb[13])();
		EXTI->PR = 1<<13;
	} else if (EXTI->PR & (1<<14)) {
		if (io_cb[14]) (io_cb[14])();
		EXTI->PR = 1<<14;
	} else if (EXTI->PR & (1<<15)) {
		if (io_cb[15]) (io_cb[15])();
		EXTI->PR = 1<<15;
	}
}

/* Definitions for EXTI configuration */
enum io_exti_mask {
	SYSCFG_EXTI_PA_MASK 	= 0x0,
	SYSCFG_EXTI_PB_MASK		= 0x1,
	SYSCFG_EXTI_PC_MASK		= 0x2,
	SYSCFG_EXTI_PD_MASK		= 0x3,
	SYSCFG_EXTI_PE_MASK		= 0x4
};

int io_configure(GPIO_TypeDef *gpio, uint16_t pin, uint16_t pin_cfg, OnIO cb) {

	// enable GPIOx subsystem clocking
	if (gpio == GPIOA) RCC->APB2ENR |= 1<<2;
	else if (gpio == GPIOB) RCC->APB2ENR |= 1<<3;
	else if (gpio == GPIOC) RCC->APB2ENR |= 1<<4;
	else if (gpio == GPIOD) RCC->APB2ENR |= 1<<5;
	else if (gpio == GPIOE) RCC->APB2ENR |= 1<<6;

	// setup pin mask for crx registers
	uint64_t pin_mask = 0;
	for(int i=0; i<16; ++i) {
		if((pin >> i) & 0x1) pin_mask |= 0x1ll << 4*i;
	}

	// clear previous data
	uint64_t crx = pin_mask * 0x4; //reset value is 0x4
	uint16_t odr = pin;
	gpio->CRH &= ~(crx >> 32);
	gpio->CRL &= ~(crx & 0xFFFFFFFF);
	gpio->BSRR |= odr << 16;

	// set up the new configuration
	crx = pin_mask * (pin_cfg & 0xF);
	odr = pin * ((pin_cfg & 0x10) >> 4);
	gpio->CRH |= crx >> 32;
	gpio->CRL |= crx & 0xFFFFFFFF;
	gpio->BSRR |= odr;

	//TODO manage alternate functions
	
	// manage IRQs //TODO allow IRQ reset
	if (!cb) return 0; //no callback attached

	if (pin_cfg & 0x7) return -1; //callback set, but not in input mode

	RCC->APB2ENR |= 0x1; //enable AFIO clocking

	// prepare mask
	uint8_t port_mask = 0;
	//	uint32_t pin_mask = 0;
	if (gpio == GPIOA) port_mask = SYSCFG_EXTI_PA_MASK;
	else if (gpio == GPIOB) port_mask = SYSCFG_EXTI_PB_MASK;
	else if (gpio == GPIOC) port_mask = SYSCFG_EXTI_PC_MASK;
	else if (gpio == GPIOD) port_mask = SYSCFG_EXTI_PD_MASK;
	else if (gpio == GPIOE) port_mask = SYSCFG_EXTI_PE_MASK;

	// setup external IRQ lines
	uint64_t afio_mask;
	for(int i=0; i<4; ++i) {
		afio_mask = ((pin_mask & (0xFFFFll << (i*16))) >> (i*16)) * port_mask;
		if(afio_mask) AFIO->EXTICR[i] |= afio_mask;
	}

	// clear pending IRQs on concerned lines
	EXTI->PR |= pin;

	// configure IRQ options and masks
	EXTI->IMR |= pin;
	if(pin_cfg & 0x100) EXTI->RTSR |= pin;
	if(pin_cfg & 0x200) EXTI->FTSR |= pin;	//in case both falling and rising
											//are set, both will be a trigger

	// register IRQ callbacks
	for(int i=0; i<16; ++i) {
		if((pin >> i) & 0x1) {
			io_cb[i] = cb;

			// Setup NVIC
			switch (i) {
				case 0:
					NVIC_SetPriority(EXTI0_IRQn, EXTI0_IRQ_PRIORITY);
					NVIC_EnableIRQ(EXTI0_IRQn);
					break;
				case 1:
					NVIC_SetPriority(EXTI1_IRQn, EXTI1_IRQ_PRIORITY);
					NVIC_EnableIRQ(EXTI1_IRQn);
					break;
				case 2:
					NVIC_SetPriority(EXTI2_IRQn, EXTI2_IRQ_PRIORITY);
					NVIC_EnableIRQ(EXTI2_IRQn);
					break;
				case 3:
					NVIC_SetPriority(EXTI3_IRQn, EXTI3_IRQ_PRIORITY);
					NVIC_EnableIRQ(EXTI3_IRQn);
					break;
				case 4:
					NVIC_SetPriority(EXTI4_IRQn, EXTI4_IRQ_PRIORITY);
					NVIC_EnableIRQ(EXTI4_IRQn);
				break;
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					NVIC_SetPriority(EXTI9_5_IRQn, EXTI9_5_IRQ_PRIORITY);
					NVIC_EnableIRQ(EXTI9_5_IRQn);
					break;
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
					NVIC_SetPriority(EXTI15_10_IRQn, EXTI15_10_IRQ_PRIORITY);
					NVIC_EnableIRQ(EXTI15_10_IRQn);
					break;
				default:
					return -1; //impossible to get there
			}
		}
	}
	return 0;
}


uint32_t io_read(GPIO_TypeDef *gpio, uint16_t mask)
{	
	return gpio->IDR & mask;	
}

void io_write(GPIO_TypeDef *gpio, uint16_t val, uint16_t mask)
{
	gpio->BSRR = (uint32_t)(mask) << (val ? 0 : 16);  
}

void io_write_n(GPIO_TypeDef *gpio, uint16_t val, uint16_t mask)
{
	gpio->BSRR = (uint32_t)(mask) << (val ? 16 : 0);  
}

void io_set(GPIO_TypeDef *gpio, uint16_t mask)
{
	gpio->BSRR = mask;  
}

void io_clear(GPIO_TypeDef *gpio, uint16_t mask)
{
	gpio->BSRR = (uint32_t)(mask) << 16;  
}
