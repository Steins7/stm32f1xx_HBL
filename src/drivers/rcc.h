#ifndef _RCC_H_
#define _RCC_H_

#include "../target/stm32f103xb.h"

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct _Clock_t {
	uint32_t	ahb_freq;
	uint32_t	apb1_freq;
	uint32_t	apb1_timer_freq;
	uint32_t	apb2_freq;
	uint32_t	apb2_timer_freq;
} Clock_t;

enum Clock_config {
	CLOCK_CONFIG_PERFORMANCE,
	CLOCK_CONFIG_POWERSAVE,
	CLOCK_CONFIG_END
};

//void SystemInit(void);

void rcc_config_clock(uint32_t config, Clock_t *sysclks);

#
#ifdef __cplusplus
}
#endif

#endif
