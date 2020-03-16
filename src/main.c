// standard headers
#include <stdint.h>
#include <stdlib.h>

// driver includes
#include "drivers/rcc.h"
#include "drivers/io.h"

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

	// configure clocks (necessary before using timers)
	rcc_config_clock(CLOCK_CONFIG_PERFORMANCE, &sysclks);

	// configure GPIO for LED
	if(io_configure(GPIOC, PIN_13, IO_MODE_OUTPUT | IO_OUT_PUSH_PULL, 0)) 
		return -1;
	io_write(GPIOC, 1, PIN_13);

	// start timed interruption
	timer_tick_init(TIM2, 1000, timeout_cb);
	timer_start(TIM2);

	uint32_t test = (uint32_t)(&end);
	test++;
	int* tab = (int*)malloc(10*sizeof(int));
	for(int i=0; i<10; ++i) {
		tab[i] = i;
	}

	// main loop
	for(;;);

	return 0;
}
