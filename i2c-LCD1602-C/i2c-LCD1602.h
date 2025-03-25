#include <stdint.h>
#include <stddef.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20 // 00100000
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04 // 00000100
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02 // 00000010
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01 // 00000001
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

struct i2c_lcd1602 {
  uint8_t address;
  uint8_t lines;
  uint8_t columns;
  uint8_t rows;
  uint8_t backlight;
};


struct i2c_lcd1602 i2c_lcd1602_init(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows);

void i2c_lcd1602_temp_func(int i2c_lcd_fd);

void i2c_lcd1602_function_set(int i2c_lcd_fd, char lines);

void i2c_lcd1602_write_4bitmode(int i2c_lcd_fd, uint8_t data, uint8_t mode);

void i2c_lcd1602_set_4bit_operation(int i2c_lcd_fd);

void i2c_lcd1602_display_clear(int i2c_lcd_fd);

void i2c_lcd1602_turn_display_on(int i2c_lcd_fd);

void i2c_lcd1602_set_entry_mode(int i2c_lcd_fd);

void i2c_lcd1602_cursor_home(int i2c_lcd_fd);





/* void autoscroll(); */
/* void backlight(); */
/* void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS ); */
/* void blink(); */
/* void clear(); */
/* void command(uint8_t); */
/* void createChar(uint8_t, uint8_t[]); */
/* void cursor(); */
/* void display(); */
/* void home(); */
/* void init(); */
/* void leftToRight(); */
/* void noAutoscroll(); */ 
/* void noBacklight(); */
/* void noBlink(); */
/* void noCursor(); */
/* void noDisplay(); */
/* void printLeft(); */
/* void printRight(); */
/* void rightToLeft(); */
/* void scrollDisplayLeft(); */
/* void scrollDisplayRight(); */
/* void setCursor(uint8_t, uint8_t); */ 
/* void shiftDecrement(); */
/* void shiftIncrement(); */
/* size_t write(uint8_t); */
/* ////compatibility API function aliases */
/* void blink_on();						// alias for blink() */
/* void blink_off();       					// alias for noBlink() */
/* void cursor_on();      	 					// alias for cursor() */
/* void cursor_off();      					// alias for noCursor() */
/* void setBacklight(uint8_t new_val);				// alias for backlight() and nobacklight() */
/* void load_custom_character(uint8_t char_num, uint8_t *rows);	// alias for createChar() */
/* void printstr(const char[]); */

/* Unsupported API functions (not implemented in this library) */
/* uint8_t status(); */
/* void setContrast(uint8_t new_val); */
/* uint8_t keypad(); */
/* void setDelay(int,int); */
/* void on(); */
/* void off(); */
/* uint8_t init_bargraph(uint8_t graphtype); */
/* void draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end); */
/* void draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end); */
	 

/* Private */
  /* void init_priv(); */
  /* void send(uint8_t, uint8_t); */
  /* void write4bits(uint8_t); */
  /* void expanderWrite(uint8_t); */
  /* void pulseEnable(uint8_t); */
  /* uint8_t _Addr; */
  /* uint8_t _displayfunction; */
  /* uint8_t _displaycontrol; */
  /* uint8_t _displaymode; */
  /* uint8_t _numlines; */
  /* uint8_t _cols; */
  /* uint8_t _rows; */
  /* uint8_t _backlightval; */
