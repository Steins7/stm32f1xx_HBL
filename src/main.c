// standard headers
#include <stdint.h>
#include <stdlib.h>

// driver includes
#include "drivers/rcc.h"
#include "drivers/io.h"
#include "drivers/flash.h"

Clock_t sysclks;
#include "drivers/timer.h"

extern uint32_t end;

//------------------------------------------------------------------------------
/* static variables */;
int val = 0; //debug led

//------------------------------------------------------------------------------
/* Timer IRQ */
static void timeout_cb(void) {
	io_write(GPIOC, val, PIN_13);
	val = !val;
}

//------------------------------------------------------------------------------
/* main function */
int main(void) {

	uint16_t* data_addr = (uint16_t*)0x8007C00; 
	uint16_t data[8] = { 0x1234, 0x2345, 0x3456, 0x4567, 
						 0x6789, 0x789A, 0x89AB, 0x9ABC };

	if(flash_erase(data_addr)) return -1;
	if(flash_write(data_addr, data, 4)) return -1;
	if(flash_erase(data_addr)) return -1;

	// configure clocks (necessary before using timers)
	rcc_config_clock(CLOCK_CONFIG_PERFORMANCE, &sysclks);

	// configure GPIO for LED
	if(io_configure(GPIOC, PIN_13, IO_MODE_OUTPUT | IO_OUT_PUSH_PULL, 0)) 
		return -1;
	io_write(GPIOC, 1, PIN_13);

	// start timed interruption
	timer_tick_init(TIM2, 2000, timeout_cb);
	timer_start(TIM2);

//	uint32_t test = (uint32_t)(&end);
//	test++;
//	int* tab = (int*)malloc(10*sizeof(int));
//	for(int i=0; i<10; ++i) {
//		tab[i] = i;
//	}

	// main loop
	for(;;);

	return 0;
}
