#ifndef FLASH_H
#define FLASH_H

//std headers
#include <stdlib.h>
#include <stdint.h>

//------------------------------------------------------------------------------

int flash_erase(uint16_t* page_addr);

int flash_write(uint16_t* addr, uint16_t* data, size_t data_len);

#endif

