#include "adc.h"

extern Clock_t sysclks;

int adc_init(ADC_TypeDef* adc) {
	// enable adc clock	
	if(adc == ADC1) RCC->APB2ENR |= 0x1 << 9;
	else if(adc == ADC2) RCC->APB2ENR |= 0x1 << 10;
	else return -1; //no such adc

	// enable adc
	adc->CR2 |= 0x1;

	// configure regular channels
	adc->CR1 = 0; //reset value
	adc->CR1 |= 0x1 << 23; //enable analog watchdog
	adc->CR1 |= 0x1 << 11; //discontinuous mode

	// set trigger to manual
	adc->CR1 |= 0x7 << 3;

	adc->SMPR2 |= 0x3FFFFFFF;

	// calibrate
	adc->CR2 |= 0x1 << 2;
	while((adc->CR2 >> 2) & 0x1);

	return 0;
}

uint16_t adc_read(ADC_TypeDef* adc, uint8_t channel) {

	adc->SQR1 &= ~(0xF << 20); //one conversion only
	adc->SQR3 = (adc->SQR3 & ~(0x1F)) | channel; //set channel

	//adc->CR2 |= 0x1 << 22; //start convertion
	adc->CR2 |= 0x1;
	while(!((adc->SR >> 1) & 0x1)); //waiting for convertion
	
	return adc->DR & 0xFFF;
	
}
