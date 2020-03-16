#include "timer.h"

extern Clock_t sysclks;

//------------------------------------------------------------------------------
static OnTick callback1 = 0;
static OnTick callback2 = 0;
static OnTick callback3 = 0;
static OnTick callback4 = 0;

void TIM1_UP_IRQHandler(void) {
	if (callback1) callback1();
	TIM1->SR &= ~0x1F;
}

void TIM2_IRQHandler(void) {
	if (callback2) callback2();
	TIM2->SR &= ~0x1F;
}

void TIM3_IRQHandler(void) {
	if (callback3) callback3();
	TIM3->SR &= ~0x1F;
}

void TIM4_IRQHandler(void) {
	if (callback4) callback4();
	TIM4->SR &= ~0x1F;
}

//------------------------------------------------------------------------------
int timer_config_cb(TIM_TypeDef* tmr, uint32_t* clk, OnTick cb) {
	IRQn_Type irqn;
	uint32_t irq_priority;

	// get clocks config
	if (tmr == TIM1) {
		*clk = sysclks.apb2_timer_freq;

		// register callback function
		callback1 = cb;
		irqn = TIM1_UP_IRQn; //every update
		irq_priority = TIM1_IRQ_PRIORITY;

		// enable timer clocking
		RCC->APB2ENR |= 1<<11;

	} else if (tmr == TIM2) {
		*clk = sysclks.apb1_timer_freq;

		// register callback function
		callback2 = cb;
		irqn = TIM2_IRQn;
		irq_priority = TIM2_IRQ_PRIORITY;

		// enable timer clocking
		RCC->APB1ENR |= 1<<0;

	} else if (tmr == TIM3) {
		*clk = sysclks.apb1_timer_freq;

		// register callback function
		callback3 = cb;
		irqn = TIM3_IRQn;
		irq_priority = TIM3_IRQ_PRIORITY;

		// enable timer clocking
		RCC->APB1ENR |= 1<<1;

	} else if (tmr == TIM4) {
		*clk = sysclks.apb1_timer_freq;

		// register callback function
		callback4 = cb;
		irqn = TIM4_IRQn;
		irq_priority = TIM4_IRQ_PRIORITY;

		// enable timer clocking
		RCC->APB1ENR |= 1<<2;
	
	} else return -1;

	// clear pending interrupts
	tmr->SR &= !1;
	
	// Enable interrupts
	tmr->DIER = (1<<0);
	NVIC_SetPriority(irqn,irq_priority);
	NVIC_EnableIRQ(irqn);

	return 0;
}

//------------------------------------------------------------------------------
int timer_wait_ms(TIM_TypeDef* tmr, uint16_t ms, OnTick cb) {
	uint32_t clk = 0;

	if(!cb) { //blocking
		//get clocks config
		if (tmr == TIM1) {
			clk = sysclks.apb2_timer_freq;
			RCC->APB2ENR |= 1<<11;
		}
		else {
			clk = sysclks.apb1_timer_freq;
			if (tmr == TIM2) RCC->APB1ENR |= 1<<0;
			else if (tmr == TIM3) RCC->APB1ENR |= 1<<1;
			else if (tmr == TIM4) RCC->APB1ENR |= 1<<2;
			else return -1;			// no such timer
		}
		
		// set period
		tmr->ARR = 0xFFFFFFFF;

	} else { //non-blocking
		if(timer_config_cb(tmr, &clk, cb)) return -1;

		// set period
		tmr->ARR = ms-1;
	}

	// set mode
	tmr->CR1 = (1<<7) | (1<<2);  //buffering and update settings
	tmr->CR1 |= (1<<3);			//one pulse mode

		// set prescaler 1ms
	tmr->PSC = 8*(clk/1000)-1; 	//PSC = clk/f - 1 | don't know why 8 times..

	timer_start(tmr);

	if(!cb) {
		while(tmr->CNT < ms); 		//waiting for end of delay
	}

	return 0;
}

int timer_wait_us(TIM_TypeDef* tmr, uint16_t us, OnTick cb) {
	uint32_t clk = 0;

	if(!cb) { //blocking
		//get clocks config
		if (tmr == TIM1) {
			clk = sysclks.apb2_timer_freq;
			RCC->APB2ENR |= 1<<11;
		}
		else {
			clk = sysclks.apb1_timer_freq;
			if (tmr == TIM2) RCC->APB1ENR |= 1<<0;
			else if (tmr == TIM3) RCC->APB1ENR |= 1<<1;
			else if (tmr == TIM4) RCC->APB1ENR |= 1<<2;
			else return -1;			// no such timer
		}

		// set period
		tmr->ARR = 0xFFFFFFFF;

	} else { //non-blocking
		if(timer_config_cb(tmr, &clk, cb)) return -1;
		
		// set period
		tmr->ARR = us-1;
	}

	// set mode
	tmr->CR1 = (1<<7) | (1<<2);  //buffering and update settings
	tmr->CR1 |= (1<<3);			//one pulse mode

	// set prescaler 1us
	tmr->PSC = 8*(clk/1000000)-1; 	//PSC = clk/f - 1 | don't know why 8 times..

	timer_start(tmr);

	if(!cb) {
		while(tmr->CNT < us); 		//waiting for end of delay
	}

	return 0;
}


//------------------------------------------------------------------------------
int timer_tick_init(TIM_TypeDef *tmr, uint16_t tick_ms, OnTick cb) {
	IRQn_Type irqn;
	uint32_t irq_priority, clk;

	// get back the clock frequency
	if (tmr == TIM1) {
		clk = sysclks.apb2_timer_freq;

		// register callback function
		callback1 = cb;
		irqn = TIM1_UP_IRQn; //every update
		irq_priority = TIM1_IRQ_PRIORITY;

		// enable timer clocking
		RCC->APB2ENR |= 1<<11;

	} else if (tmr == TIM2) {
		clk = sysclks.apb1_timer_freq;

		// register callback function
		callback2 = cb;
		irqn = TIM2_IRQn;
		irq_priority = TIM2_IRQ_PRIORITY;

		// enable timer clocking
		RCC->APB1ENR |= 1<<0;

	} else if (tmr == TIM3) {
		clk = sysclks.apb1_timer_freq;

		// register callback function
		callback3 = cb;
		irqn = TIM3_IRQn;
		irq_priority = TIM3_IRQ_PRIORITY;

		// enable timer clocking
		RCC->APB1ENR |= 1<<1;

	} else if (tmr == TIM4) {
		clk = sysclks.apb1_timer_freq;

		// register callback function
		callback4 = cb;
		irqn = TIM4_IRQn;
		irq_priority = TIM4_IRQ_PRIORITY;

		// enable timer clocking
		RCC->APB1ENR |= 1<<2;
	
	} else return -1;

	// clear pending interrupts
	tmr->SR &= !1;

	// set mode
	tmr->CR1 = (1<<7) | (1<<2);  //buffering and update settings
	tmr->DIER = (1<<0); //Enable interrupts

	// set prescaler 1ms
	tmr->PSC = 8*(clk/1000)-1; //PSC = clk/f - 1 | don't know why 8 times...

	// set period
	if(timer_set_period(tmr,tick_ms)) return -1; 

	if (cb) {
		NVIC_SetPriority(irqn,irq_priority);
		NVIC_EnableIRQ(irqn);					//unmask IRQ
	}

	return 0;
}

int timer_set_period(TIM_TypeDef *tmr, uint16_t tick) {
	// set period
	tmr->ARR = tick-1; //tickms = (ARR+1)Tpsc

	// force update to reset counter and apply prescaler
	tmr->EGR |= 1;
	return 0;
}

void timer_start(TIM_TypeDef *tmr) {
	// force update to reset counter and prescaler
	tmr->EGR |= 1;

	// enable counting
	tmr->CR1 |= 1;
}

void timer_stop(TIM_TypeDef *tmr) {
	// disable counting
	tmr->CR1 &= !1;
}

//------------------------------------------------------------------------------
int timer_enc_init(TIM_TypeDef* tmr) {
	// enable timer
	if (tmr == TIM1) {
		RCC->APB2ENR |= 1<<11;

	} else if (tmr == TIM2) {
		RCC->APB1ENR |= 1<<0;

	} else if (tmr == TIM3) {
		RCC->APB1ENR |= 1<<1;

	} else if (tmr == TIM4) {
		RCC->APB1ENR |= 1<<2;
	
	} else return -1; //no such timer

	//TODO set registers at reset value
	
	tmr->SMCR |= 0x1; //count on only one edge
	tmr->ARR = (1 << 16)-1; 

	// map inputs
	tmr->CCMR1 |= 0x9;
	tmr->CCMR1 |= 0x9 << 8;
	
	// enable input channels and invert them //TODO add an otpion for that
	tmr->CCER |= 0x3;
	tmr->CCER |= 0x3 << 4;

	tmr->CR1 |= 0x1; //enable timer

	return 0;
}

