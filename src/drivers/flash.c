//driver header
#include "flash.h"

//target header
#include "../target/stm32f103xb.h"

//custom header
#include "../config.h"

int unlock(void) {
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;

	if(FLASH->CR & (1<<7)) return -1;
	return 0;
}

void lock(void) {
	FLASH->CR |= 1<<7;
}

int flash_erase(uint16_t* page_addr) {
	if(unlock()) return -1;

	// check that flash is not busy
	if(FLASH->SR & 1<<0) {
		lock();
		return -1;
	}
	
	// select the page to erase
	FLASH->CR |= 1<<1;
	FLASH->AR = (uint32_t)page_addr;

	// start erase
	FLASH->CR |= 1<<6;

	// wait for the operation to finish
	while(FLASH->SR & 1<<0);
	FLASH->CR &= ~(1<<1);
	lock();

	// check for errors
	if(!(FLASH->SR & 1<<5)) return -1;
	FLASH->SR |= 1<<5;

	return 0;
}

int flash_write(uint16_t* addr, uint16_t* data, size_t data_len) {
	if(unlock()) return -1;

	// check that flash is not busy
	if(FLASH->SR & 1<<0) return -1;

	// write the flash
	for(size_t i=0; i<data_len; ++i) {
		FLASH->CR |= 1<<0;
		addr[i] = data[i];
		while(FLASH->SR & 1<<0);
	}

	// finish write operation
	FLASH->CR &= ~(1<<0);
	lock();

	// check for errors
	if(!(FLASH->SR & 1<<5)) return -1;
	FLASH->SR |= 1<<5;

	for(size_t i=0; i<data_len; ++i) {
		if(addr[i] != data[i]) return -1;	
	}

	return 0;
}

