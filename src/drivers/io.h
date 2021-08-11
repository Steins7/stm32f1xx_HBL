#ifndef IO_H
#define IO_H

//std headers
#include <stdlib.h>
#include <stdint.h>

//target header
#include "../target/stm32f103xb.h"

//custom header
#include "../config.h"

//------------------------------------------------------------------------------
/* GPIO pin mask definitions */
enum io_pin {
	PIN_0					= (1 << 0),
	PIN_1					= (1 << 1),
	PIN_2					= (1 << 2),
	PIN_3					= (1 << 3),
	PIN_4					= (1 << 4),
	PIN_5					= (1 << 5),
	PIN_6					= (1 << 6),
	PIN_7					= (1 << 7),
	PIN_8					= (1 << 8),
	PIN_9					= (1 << 9),
	PIN_10					= (1 << 10),
	PIN_11					= (1 << 11),
	PIN_12					= (1 << 12),
	PIN_13					= (1 << 13),
	PIN_14					= (1 << 14),
	PIN_15					= (1 << 15),
	PIN_ALL					= 0xFFFF
};

//------------------------------------------------------------------------------
/* GPIO pin mode definitions */
enum io_mode {
	IO_MODE_INPUT 			= (0x0),
	IO_MODE_OUTPUT 			= (0x1), // 10Mhz max
	IO_MODE_OUTPUT_SLOW 	= (0x3), // 2MHz max
	IO_MODE_OUTPUT_FAST 	= (0x4)  // 50 MHz max
};

//------------------------------------------------------------------------------
/* GPIO pin conf definitions */
enum io_conf {
	IO_IN_ANALOG			= (0b0   << 2),
	IO_IN_FLOATING			= (0b1   << 2),
	IO_IN_PULL_UP			= (0b110 << 2),
	IO_IN_PULL_DOWN			= (0b010 << 2),
	IO_OUT_ALT_FNCT			= (0b10  << 2),
	IO_OUT_PUSH_PULL		= (0b0   << 2),
	IO_OUT_OPEN_DRAIN		= (0b1   << 2)
};

//------------------------------------------------------------------------------
/* GPIO pin clear */
#define IO_CLEAR			(0)

//------------------------------------------------------------------------------
/* alternate function selection option */
//TODO not supported for now
//enum io_alt_fnct {
//	PIN_OPT_AF0				0x0,
//	PIN_OPT_AF1				0x1,
//	PIN_OPT_AF2				0x2,
//	PIN_OPT_AF3				0x3,
//	PIN_OPT_AF4				0x4,
//	PIN_OPT_AF5				0x5,
//	PIN_OPT_AF6				0x6,
//	PIN_OPT_AF7				0x7,
//	PIN_OPT_AF8				0x8,
//	PIN_OPT_AF9				0x9,
//	PIN_OPT_AF10			0xA,
//	PIN_OPT_AF11			0xB,
//	PIN_OPT_AF12			0xC,
//	PIN_OPT_AF13			0xD,
//	PIN_OPT_AF14			0xE,
//	PIN_OPT_AF15			0xF
//};

//------------------------------------------------------------------------------
/* GPIO IRQ conf definitons */
enum io_irq_conf {
	IO_IRQ_EDGE_RISE	= (0x1 << 8),
	IO_IRQ_EDGE_FALL	= (0x2 << 8),
	IO_IRQ_EDGE_BOTH	= (0x3 << 8)
};

typedef void (*OnIO)();

//------------------------------------------------------------------------------
/* io_configure
 *
 * configure pins referenced in 'pin_mask' of specified port
 * 'gpio' according to 'pin_cfg' and associate a callback
 * function 'cb' if not NULL.
 * returns 0 if success
 */
int io_configure(GPIO_TypeDef *gpio, uint16_t pin_mask, uint16_t pin_cfg, 
		OnIO cb);

/* io_read
 *
 * read 32 bit data from port 'gpio', filter the result with mask
 */
uint32_t io_read(GPIO_TypeDef *gpio, uint16_t mask);

/* io_write
 *
 * write 16 bit data filtered by mask to port 'gpio'
 * '1' in val are written as HIGH level on port pins
 */
void io_write(GPIO_TypeDef *gpio, uint16_t val, uint16_t mask);

/* io_write_n
 *
 * write 16 bit data filtered by mask to port 'gpio'
 * '1' in val are written as LOW level on port pins
 */
void io_write_n(GPIO_TypeDef *gpio, uint16_t val, uint16_t mask);

/* io_set/clear
 *
 * set or clear outputs according to bit mask
 */
void io_set(GPIO_TypeDef *gpio, uint16_t mask);
void io_clear(GPIO_TypeDef *gpio, uint16_t mask);

#endif

