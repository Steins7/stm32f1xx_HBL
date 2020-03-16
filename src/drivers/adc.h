#ifndef ADC_H
#define ADC_H

#include "../target/stm32f103xb.h"
#include "../config.h"
#include "rcc.h"


int adc_init(ADC_TypeDef* adc);

uint16_t adc_read(ADC_TypeDef* adc, uint8_t channel);


#endif

