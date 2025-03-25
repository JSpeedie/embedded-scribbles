#include <inttypes.h>
#include <time.h>
#include <unistd.h>

#include "i2c-LCD1602.h"


struct i2c_lcd1602 i2c_lcd1602_init(uint8_t lcd_addr, uint8_t lcd_columns,
	uint8_t lcd_rows) {

	struct i2c_lcd1602 ret = {
		.address = lcd_addr,
		.columns = lcd_columns,
		.rows = lcd_rows,
		.backlight = LCD_NOBACKLIGHT
	};

	return ret;
}


void i2c_lcd1602_begin(int i2c_lcd_fd, uint8_t cols, uint8_t lines, uint8_t
	dotsize) {

	/* According to the Hitachi HD44780U datasheet
	* (https://www.sparkfun.com/datasheets/LCD/HD44780.pdf),
	* when the display powers up, it is configured as follows:
	*
	* 1. Display clear
	* 2. Function set:
	*    DL = 1; 8-bit interface data
	*    N = 0; 1-line display
	*    F = 0; 5x8 dot character font
	* 3. Display on/off control:
	*    D = 0; Display off
	*    C = 0; Cursor off
	*    B = 0; Blinking off
	* 4. Entry mode set:
	*    I/D = 1; Increment by 1
	*    S = 0; No shift
	* This function exists then to change those values to what could be
	* common-case defaults.
	*/


	/* See page 46 of the HD44780U datasheet for 4-bit initialization
	 * specifictation: https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
	 * According to the datasheet, we must wait for 40ms after the Vcc reaches
	 * 2.7 V before sending commands. */
	/* Sleep for 50ms */
	struct timespec a = { .tv_sec = 0, .tv_nsec = 50000000};
	nanosleep(&a, NULL);

	/* See page 46 of the HD44780U datasheet for 4-bit initialization
	 * specifictation: https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
	 * According to the datasheet, there are 3 function sets before other
	 * instructions can be sent */
	/* TODO: temp func that does the first 3 function set calls */
	i2c_lcd1602_temp_func(i2c_lcd_fd);
	// finally, set to 4-bit interface (me: perhaps this 0x02 is the correct value for turning on 4bit operation?)
	i2c_lcd1602_set_4bit_operation(i2c_lcd_fd);

	/* Set the display function of the LCD  (i.e. the number of lines, font
	 * size, etc.) */
	i2c_lcd1602_function_set(i2c_lcd_fd, 1);

	/* Set the display control of the LCD  (E.g. here: display = on) */
	i2c_lcd1602_turn_display_on(i2c_lcd_fd);

	/* Clear the display */
	i2c_lcd1602_display_clear(i2c_lcd_fd);

	/* Set the LCD to have new characters enter from what should be the default
	 * text direction for roman languages (i.e., new characters should enter
	 * from the left) */
	i2c_lcd1602_set_entry_mode(i2c_lcd_fd);

	/* Move the cursor back to the beginning */
	/* i2c_lcd1602_cursor_home(i2c_lcd_fd); */
}


/** Write to the i2c LCD in 4 bit mode */
void i2c_lcd1602_write_4bitmode(int i2c_lcd_fd, uint8_t data, uint8_t mode) {
	uint8_t highnib = (data & 0xf0) | mode;
	uint8_t lownib = ((data << 4) & 0xf0) | mode;
	write(i2c_lcd_fd, &highnib, 1);
	write(i2c_lcd_fd, &lownib, 1);
}


/** ??? */
void i2c_lcd1602_temp_func(int i2c_lcd_fd) {
	/* See page 46 of the HD44780U datasheet:
	* https://www.sparkfun.com/datasheets/LCD/HD44780.pdf */

	/* Set DB7 to DB4 appropriately */
	uint8_t data = 0x20; // 00110000
	/* Set RS and R/W appropriately */
	uint8_t mode = 0x00; // 00000000

	i2c_lcd1602_write_4bitmode(i2c_lcd_fd, data, mode);

	/* According to page 46 of the HD44780U datasheet
	 * (https://www.sparkfun.com/datasheets/LCD/HD44780.pdf),
	 * this instruction must be followed by a wait that is at least 4.1ms */
	/* Sleep for 4.5ms */
	struct timespec a = (struct timespec) { .tv_sec = 0, .tv_nsec = 4500000};
	nanosleep(&a, NULL);

	i2c_lcd1602_write_4bitmode(i2c_lcd_fd, data, mode);

	/* According to page 46 of the HD44780U datasheet
	 * (https://www.sparkfun.com/datasheets/LCD/HD44780.pdf),
	 * this instruction must be followed by a wait that is at least 100µs */
	/* Sleep for 150µs */
	a = (struct timespec) { .tv_sec = 0, .tv_nsec = 150000};
	nanosleep(&a, NULL);

	i2c_lcd1602_write_4bitmode(i2c_lcd_fd, data, mode);
}


/** Set the i2c LCD to 4 bit mode */
void i2c_lcd1602_set_4bit_operation(int i2c_lcd_fd) {
	/* See page 42 of the HD44780U datasheet:
	* https://www.sparkfun.com/datasheets/LCD/HD44780.pdf */

	/* Set the command type */
	uint8_t data = LCD_FUNCTIONSET;
	/* Set the details of the command, i.e., set the data length to 4 bits
	 * (i.e. set 4-bit mode) */
	data |= LCD_4BITMODE;
	/* Set RS and R/W appropriately */
	uint8_t mode = 0x00; // 00000000

	i2c_lcd1602_write_4bitmode(i2c_lcd_fd, data, mode);

	/* According to page 46 of the HD44780U datasheet
	 * (https://www.sparkfun.com/datasheets/LCD/HD44780.pdf),
	 * this operation takes a minimum of 4.1ms */
	/* Sleep for 4.5ms */
	struct timespec a = (struct timespec) { .tv_sec = 0, .tv_nsec = 4500000};
	nanosleep(&a, NULL);
}


/** Clear the display, and set the cursor position to zero */
void i2c_lcd1602_display_clear(int i2c_lcd_fd) {
	/* See page 28 of the HD44780U datasheet:
	* https://www.sparkfun.com/datasheets/LCD/HD44780.pdf */

	/* Set DB7 to DB0 appropriately */
	uint8_t data = LCD_CLEARDISPLAY;
	/* Set RS and R/W (respectively) to binary 00 */
	uint8_t mode = 0x00; // 00000000

	i2c_lcd1602_write_4bitmode(i2c_lcd_fd, data, mode);

	// this command takes a long time! */
	/* Sleep for 2000µs */
	struct timespec a = (struct timespec) { .tv_sec = 0, .tv_nsec = 2000000};
	nanosleep(&a, NULL);
}


/** Establish the functionality settings for the i2c LCD */
void i2c_lcd1602_function_set(int i2c_lcd_fd, char lines) {
	/* See page 24 of the HD44780U datasheet:
	 * https://www.sparkfun.com/datasheets/LCD/HD44780.pdf */

	/* Update struct to reflect the number of lines in this i2c LCD instance */
	/* i2c_lcd.lines = lines; */
	/* Update struct to reflect the number of lines in this i2c LCD instance */
	/* i2c_lcd.displayfunction = ? LCD_5x10DOTS; */

	/* Set the command type */
	uint8_t data = LCD_FUNCTIONSET;
	if (lines > 1) data |= LCD_2LINE;
	// TODO: before function setting to 5x10 chars, it should first be
	// checked that the LCD display supports it
	data |= LCD_5x10DOTS;
	/* Set RS and R/W (respectively) to binary 00 */
	uint8_t mode = 0x00; // 00000000

	i2c_lcd1602_write_4bitmode(i2c_lcd_fd, data, mode);

	/* Sleep for 50µs */
	struct timespec a = (struct timespec) { .tv_sec = 0, .tv_nsec = 50000};
	nanosleep(&a, NULL);
}


/** Set the display controls for the i2c LCD */
void i2c_lcd1602_turn_display_on(int i2c_lcd_fd) {
	/* See page 42 of the HD44780U datasheet:
	 * https://www.sparkfun.com/datasheets/LCD/HD44780.pdf */

	/* Set DB7 to DB0, i.e., set: the display to be on, the cursor to be on,
	 * and the cursor blink to be on */
	uint8_t data = (LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);
	/* Set RS and R/W (respectively) to binary 00 */
	uint8_t mode = 0x00; // 00000000

	i2c_lcd1602_write_4bitmode(i2c_lcd_fd, data, mode);

	/* Sleep for 50µs */
	struct timespec a = (struct timespec) { .tv_sec = 0, .tv_nsec = 50000};
	nanosleep(&a, NULL);
}


/** Set the entry mode for the i2c LCD */
void i2c_lcd1602_set_entry_mode(int i2c_lcd_fd) {
	/* See page 42 of the HD44780U datasheet:
	 * https://www.sparkfun.com/datasheets/LCD/HD44780.pdf */

	/* Set the command type */
	uint8_t data = LCD_ENTRYMODESET;
	/* Set the details of the command, i.e., set the entry mode (E.g. here:
	 * have characters enter from the left, and shift decrement) */
	data |= (LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
	/* Set RS and R/W (respectively) to binary 00 */
	uint8_t mode = 0x00; // 00000000

	i2c_lcd1602_write_4bitmode(i2c_lcd_fd, data, mode);

	/* Sleep for 50µs */
	struct timespec a = (struct timespec) { .tv_sec = 0, .tv_nsec = 50000};
	nanosleep(&a, NULL);
}


void i2c_lcd1602_cursor_home(int i2c_lcd_fd) {
	/* See page 24 of the HD44780U datasheet:
	 * https://www.sparkfun.com/datasheets/LCD/HD44780.pdf */

	/* Set DB7 to DB0 appropriately */
	uint8_t data = LCD_RETURNHOME;
	/* Set RS and R/W (respectively) to binary 00 */
	uint8_t mode = 0x00; // 00000000

	i2c_lcd1602_write_4bitmode(i2c_lcd_fd, data, mode);

	/* Sleep for 1.52ms */
	struct timespec a = (struct timespec) { .tv_sec = 0, .tv_nsec = 1520000};
	nanosleep(&a, NULL);
}

/* void i2c_lcd1602_set_cursor_loc(uint8_t col, uint8_t row) { */
/* 	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 }; */
/* 	if ( row > _numlines ) { */
/* 		row = _numlines - 1;    // we count rows starting w/0 */
/* 	} */
/* 	command(LCD_SETDDRAMADDR | (col + row_offsets[row])); */
/* } */

/* // Turn the display on/off (quickly) */
/* void LiquidCrystal_I2C::noDisplay() { */
/* 	_displaycontrol &= ~LCD_DISPLAYON; */
/* 	command(LCD_DISPLAYCONTROL | _displaycontrol); */
/* } */
/* void LiquidCrystal_I2C::display() { */
/* 	_displaycontrol |= LCD_DISPLAYON; */
/* 	command(LCD_DISPLAYCONTROL | _displaycontrol); */
/* } */

/* // Turns the underline cursor on/off */
/* void LiquidCrystal_I2C::noCursor() { */
/* 	_displaycontrol &= ~LCD_CURSORON; */
/* 	command(LCD_DISPLAYCONTROL | _displaycontrol); */
/* } */
/* void LiquidCrystal_I2C::cursor() { */
/* 	_displaycontrol |= LCD_CURSORON; */
/* 	command(LCD_DISPLAYCONTROL | _displaycontrol); */
/* } */

/* // Turn on and off the blinking cursor */
/* void LiquidCrystal_I2C::noBlink() { */
/* 	_displaycontrol &= ~LCD_BLINKON; */
/* 	command(LCD_DISPLAYCONTROL | _displaycontrol); */
/* } */
/* void LiquidCrystal_I2C::blink() { */
/* 	_displaycontrol |= LCD_BLINKON; */
/* 	command(LCD_DISPLAYCONTROL | _displaycontrol); */
/* } */

/* // These commands scroll the display without changing the RAM */
/* void LiquidCrystal_I2C::scrollDisplayLeft(void) { */
/* 	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT); */
/* } */
/* void LiquidCrystal_I2C::scrollDisplayRight(void) { */
/* 	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT); */
/* } */

/* // This is for text that flows Left to Right */
/* void LiquidCrystal_I2C::leftToRight(void) { */
/* 	_displaymode |= LCD_ENTRYLEFT; */
/* 	command(LCD_ENTRYMODESET | _displaymode); */
/* } */

/* // This is for text that flows Right to Left */
/* void LiquidCrystal_I2C::rightToLeft(void) { */
/* 	_displaymode &= ~LCD_ENTRYLEFT; */
/* 	command(LCD_ENTRYMODESET | _displaymode); */
/* } */

/* // This will 'right justify' text from the cursor */
/* void LiquidCrystal_I2C::autoscroll(void) { */
/* 	_displaymode |= LCD_ENTRYSHIFTINCREMENT; */
/* 	command(LCD_ENTRYMODESET | _displaymode); */
/* } */

/* // This will 'left justify' text from the cursor */
/* void LiquidCrystal_I2C::noAutoscroll(void) { */
/* 	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT; */
/* 	command(LCD_ENTRYMODESET | _displaymode); */
/* } */

/* // Allows us to fill the first 8 CGRAM locations */
/* // with custom characters */
/* void LiquidCrystal_I2C::createChar(uint8_t location, uint8_t charmap[]) { */
/* 	location &= 0x7; // we only have 8 locations 0-7 */
/* 	command(LCD_SETCGRAMADDR | (location << 3)); */
/* 	for (int i=0; i<8; i++) { */
/* 		write(charmap[i]); */
/* 	} */
/* } */

/* // Turn the (optional) backlight off/on */
/* void LiquidCrystal_I2C::noBacklight(void) { */
/* 	_backlightval=LCD_NOBACKLIGHT; */
/* 	expanderWrite(0); */
/* } */

/* void LiquidCrystal_I2C::backlight(void) { */
/* 	_backlightval=LCD_BACKLIGHT; */
/* 	expanderWrite(0); */
/* } */



/*********** mid level commands, for sending data/cmds */

/* inline void LiquidCrystal_I2C::command(uint8_t value) { */
/* 	send(value, 0); */
/* } */


/************ low level data pushing commands **********/

/* // write either command or data */
/* void LiquidCrystal_I2C::send(uint8_t value, uint8_t mode) { */
/* 	uint8_t highnib=value&0xf0; */
/* 	uint8_t lownib=(value<<4)&0xf0; */
/*        write4bits((highnib)|mode); */
/* 	write4bits((lownib)|mode); */
/* } */

/* void LiquidCrystal_I2C::write4bits(uint8_t value) { */
/* 	expanderWrite(value); */
/* 	pulseEnable(value); */
/* } */

/* void LiquidCrystal_I2C::expanderWrite(uint8_t _data){ */
/* 	Wire.beginTransmission(_Addr); */
/* 	printIIC((int)(_data) | _backlightval); */
/* 	Wire.endTransmission(); */
/* } */

/* void LiquidCrystal_I2C::pulseEnable(uint8_t _data){ */
/* 	expanderWrite(_data | En);	// En high */
/* 	delayMicroseconds(1);		// enable pulse must be >450ns */

/* 	expanderWrite(_data & ~En);	// En low */
/* 	delayMicroseconds(50);		// commands need > 37us to settle */
/* } */


/* // Alias functions */

/* void LiquidCrystal_I2C::cursor_on(){ */
/* 	cursor(); */
/* } */

/* void LiquidCrystal_I2C::cursor_off(){ */
/* 	noCursor(); */
/* } */

/* void LiquidCrystal_I2C::blink_on(){ */
/* 	blink(); */
/* } */

/* void LiquidCrystal_I2C::blink_off(){ */
/* 	noBlink(); */
/* } */

/* void LiquidCrystal_I2C::load_custom_character(uint8_t char_num, uint8_t *rows){ */
/* 		createChar(char_num, rows); */
/* } */

/* void LiquidCrystal_I2C::setBacklight(uint8_t new_val){ */
/* 	if(new_val){ */
/* 		backlight();		// turn backlight on */
/* 	}else{ */
/* 		noBacklight();		// turn backlight off */
/* 	} */
/* } */

/* void LiquidCrystal_I2C::printstr(const char c[]){ */
/* 	//This function is not identical to the function used for "real" I2C displays */
/* 	//it's here so the user sketch doesn't have to be changed */
/* 	print(c); */
/* } */


/* // unsupported API functions */
/* void LiquidCrystal_I2C::off(){} */
/* void LiquidCrystal_I2C::on(){} */
/* void LiquidCrystal_I2C::setDelay (int cmdDelay,int charDelay) {} */
/* uint8_t LiquidCrystal_I2C::status(){return 0;} */
/* uint8_t LiquidCrystal_I2C::keypad (){return 0;} */
/* uint8_t LiquidCrystal_I2C::init_bargraph(uint8_t graphtype){return 0;} */
/* void LiquidCrystal_I2C::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end){} */
/* void LiquidCrystal_I2C::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_row_end){} */
/* void LiquidCrystal_I2C::setContrast(uint8_t new_val){} */


