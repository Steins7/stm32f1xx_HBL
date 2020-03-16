#ifndef LCD_H
#define LCD_H

#include "../target/stm32f103xb.h"
#include "../config.h"
#include "timer.h"
#include "io.h"
#include <stdlib.h>

//------------------------------------------------------------------------------
/* LCD mode selection */
enum lcd_register {
	LCD_MODE_CMD		= 0,
	LCD_MODE_DATA		= 1
};

//------------------------------------------------------------------------------
/* LCD commands */
enum lcd_command {
	LCD_CLEAR			= 0x01,
	LCD_CUR_HOME		= 0x02,
	LCD_ENTRY			= 0x04,
	LCD_DISP_CTRL		= 0x08,
	LCD_SHIFT			= 0x10,
	LCD_FUNC_SET		= 0x20,
	LCD_CGRAM_ADDR		= 0x40,
	LCD_DDRAM_ADDR		= 0x80
};

//------------------------------------------------------------------------------
/* LCD_ENTRY command options */
enum lcd_entry_option {
	LCD_ENTRY_RIGHT 	= 0x00,
	LCD_ENTRY_LEFT 		= 0x02,
	LCD_ENTRY_SHIFT_INC = 0x01,
	LCD_ENTRY_SHIFT_DEC = 0x00
};

//------------------------------------------------------------------------------
/* LCD_DISP_CTRL command options */
enum lcd_ctrl_option {
	LCD_CTRL_DISP_ON 	= 0x04,
	LCD_CTRL_DISP_OFF 	= 0x00,
	LCD_CTRL_CUR_ON 	= 0x02,
	LCD_CTRL_CUR_OFF 	= 0x00,
	LCD_CTRL_BLINK_ON 	= 0x01,
	LCD_CTRL_BLINK_OFF 	= 0x00
};

//------------------------------------------------------------------------------
/* LCD_SHIFT command options */
enum lcd_shift_option {
	LCD_SHIFT_DISP		= 0x08,
	LCD_SHIFT_CUR		= 0x00,
	LCD_SHIFT_RIGHT		= 0x04,
	LCD_SHIFT_LEFT		= 0x00
};

//------------------------------------------------------------------------------
/* LCD_FUNC_SET command options */
enum lcd_func_option {
	LCD_FUNC_8BIT 		= 0x10,
	LCD_FUNC_4BIT 		= 0x00,
	LCD_FUNC_2LINE 		= 0x08,
	LCD_FUNC_1LINE 		= 0x00,
	LCD_FUNC_5x10DOTS 	= 0x04,
	LCD_FUNC_5x8DOTS 	= 0x00
};

//------------------------------------------------------------------------------
/** lcd_init
 * 	initialise the lcd, needed before anything else can be done
 * 	the timer is used for delays and can't be in use when lcd functions are 
 * 	called
 */
int lcd_init(TIM_TypeDef* tim, uint8_t col, uint8_t row);

/** lcd_send_cmd
 * 	send the specified command to the lcd
 */
void lcd_send_cmd(uint8_t cmd);

/** lcd_print
 *  print a null-terminated string on the lcd
 */
void lcd_print(const char* txt);

void lcd_print_c(char c);

void lcd_set_cursor(uint8_t col, uint8_t row);

#endif

