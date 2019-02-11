#include "LCD_I2C.h"
#include <inttypes.h>
#include <Arduino.h>
#include <Wire.h>

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LCD_I2C::LCD_I2C(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows, uint8_t charsize)
{
	_addr = lcd_addr;
	_cols = lcd_cols;
	_rows = lcd_rows;
	_charsize = charsize;
	_backlightval = LCD_BACKLIGHT;
}

void LCD_I2C::begin() {
	Wire.begin();
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if (_rows > 1) {
		_displayfunction |= LCD_2LINE;
	}

	// for some 1 line displays you can select a 10 pixel high font
	if ((_charsize != 0) && (_rows == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	delay(50);

	// Now we pull both RS and R/W low to begin commands
	expanderWrite(_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	delay(1000);

	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03 << 4);
	delayMicroseconds(4500); // wait min 4.1ms

	// second try
	write4bits(0x03 << 4);
	delayMicroseconds(4500); // wait min 4.1ms

	// third go!
	write4bits(0x03 << 4);
	delayMicroseconds(150);

	// finally, set to 4-bit interface
	write4bits(0x02 << 4);

	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	// clear it off
	clear();

	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);

	home();
}

/********** high level commands, for the user! */
void LCD_I2C::clear(){
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void LCD_I2C::home(){
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void LCD_I2C::setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > _rows) {
		row = _rows-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCD_I2C::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_I2C::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LCD_I2C::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_I2C::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LCD_I2C::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_I2C::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCD_I2C::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCD_I2C::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCD_I2C::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LCD_I2C::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LCD_I2C::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LCD_I2C::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCD_I2C::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
		write(charmap[i]);
	}
}

// Turn the (optional) backlight off/on
void LCD_I2C::noBacklight(void) {
	_backlightval=LCD_NOBACKLIGHT;
	expanderWrite(0);
}

void LCD_I2C::backlight(void) {
	_backlightval=LCD_BACKLIGHT;
	expanderWrite(0);
}
bool LCD_I2C::getBacklight() {
  return _backlightval == LCD_BACKLIGHT;
}


/*********** mid level commands, for sending data/cmds */

inline void LCD_I2C::command(uint8_t value) {
	send(value, 0);
}

inline size_t LCD_I2C::write(uint8_t value) {
	send(value, Rs);
	return 1;
}


/************ low level data pushing commands **********/

// write either command or data
void LCD_I2C::send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
	write4bits((highnib)|mode);
	write4bits((lownib)|mode);
}

void LCD_I2C::write4bits(uint8_t value) {
	expanderWrite(value);
	pulseEnable(value);
}

void LCD_I2C::expanderWrite(uint8_t _data){
	Wire.beginTransmission(_addr);
	Wire.write((int)(_data) | _backlightval);
	Wire.endTransmission();
}

void LCD_I2C::pulseEnable(uint8_t _data){
	expanderWrite(_data | En);	// En high
	delayMicroseconds(1);		// enable pulse must be >450ns

	expanderWrite(_data & ~En);	// En low
	delayMicroseconds(50);		// commands need > 37ms to settle
}

/************ Additional functionality *****************/

void LCD_I2C::customClear() {
	unsigned char blank[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	for (int i = 0; i < 8; i += 1) {
		createChar(i, blank);
	}
}

void LCD_I2C::printBar(uint8_t x, uint8_t y, uint8_t len, uint8_t value, uint8_t style = BAR_BORDERS) {
	uint8_t last_x = x + len - 1;
	if (!value) {
		value = 1;
	}

	switch (style)
	{
	case BAR_BORDERS:
		custom_set_0();
		break;
	case BAR_NOBORDERS:
		custom_set_4();
		break;
	}
	setCursor(x, y);

	for (int i = x; i < last_x; i += 1) {
		if (value >= 5) {
			write(255);
			value -= 5;
		}
		else {
			write(value);
			value = 0;
		}
	}

	if (style == BAR_BORDERS) {
		if (value >= 4) {
			write(255);
		}
		else if (!value) {
			write(7);
		}
		else {
			setCursor(last_x, y);
			write(value);
		}
	}
	else {
		if (value >= 5) {
			write(255);
		}
		else {
			write(value);
		}
	}
}
void LCD_I2C::makePlot(uint8_t x, uint8_t y, uint8_t len, uint8_t height, uint8_t values[], uint8_t style = PLOT_FILLED) {
	y += height - 1;
	switch (style) {
	case PLOT_FILLED:
		custom_set_2();
		for (int col = 0; col < len; col += 1) {
			print_col_0(x + col, y, height, values[col]);
		}
		break;
	case PLOT_UNFILLED:
		custom_set_3();
		for (int col = 0; col < len; col += 1) {
			print_col_1(x + col, y, height, values[col]);
		}
		break;
	}
	
}

void LCD_I2C::print_col_0(uint8_t x, uint8_t y, uint8_t height, uint8_t val) {
	for (int i = 0; i < height; i += 1) {
		setCursor(x, y - i);
		if (val >= 8) {
			write(255);
			val -= 8;
		}
		else {
			write(val);
			val = 0;
		}
	}
}
void LCD_I2C::print_col_1(uint8_t x, uint8_t y, uint8_t height, uint8_t val) {
	for (int i = 0; i < height; i += 1) {
		setCursor(x, y - i);
		if (val == 0 || val > 8) {
			print(' ');
			val -= 8;
		}
		else {
			write(val - 1);
			val = 0;
		}
	}
}

/************ Custom sets ******************************/

void LCD_I2C::custom_set_0() {
	if (_cur_custom_set != 0) {
		_cur_custom_set = 0;

		unsigned char f0[8] = { -1, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, -1 };
		unsigned char f1[8] = { -1, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, -1 };
		unsigned char f2[8] = { -1, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, -1 };
		unsigned char f3[8] = { -1, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, -1 };
		unsigned char f4[8] = { -1, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, -1 };
		unsigned char most_right[8] = { -1, 1, 1, 1, 1, 1, 1, -1 };

		createChar(0, f0);
		createChar(1, f1);
		createChar(2, f2);
		createChar(3, f3);
		createChar(4, f4);
		createChar(7, most_right);
	}
}
void LCD_I2C::custom_set_1() {
	if (_cur_custom_set != 1) {
		_cur_custom_set = 1;
		customClear();

		unsigned char f0[8] = { -1, 1, 1, 1, 1, 1, 1, -1 };
		unsigned char f1[8] = { -1, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, -1 };
		unsigned char f2[8] = { -1, 0b11001, 0b11001, 0b11001, 0b11001, 0b11001, 0b11001, -1 };
		unsigned char f3[8] = { -1, 0b11101, 0b11101, 0b11101, 0b11101, 0b11101, 0b11101, -1 };

		createChar(7, f0);
		createChar(1, f1);
		createChar(2, f2);
		createChar(3, f3);
	}
}
void LCD_I2C::custom_set_4() {
	if (_cur_custom_set != 4) {
		_cur_custom_set = 4;
		customClear();

		unsigned char f1[8] = { 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000 };
		unsigned char f2[8] = { 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000 };
		unsigned char f3[8] = { 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100 };
		unsigned char f4[8] = { 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110 };

		createChar(1, f1);
		createChar(2, f2);
		createChar(3, f3);
		createChar(4, f4);
	}
}

void LCD_I2C::custom_set_2() {
	if (_cur_custom_set != 2) {
		_cur_custom_set = 2;
		customClear();

		unsigned char f1[8] = { 0,  0,  0,  0,  0,  0,  0, -1 };
		unsigned char f2[8] = { 0,  0,  0,  0,  0,  0, -1, -1 };
		unsigned char f3[8] = { 0,  0,  0,  0,  0, -1, -1, -1 };
		unsigned char f4[8] = { 0,  0,  0,  0, -1, -1, -1, -1 };
		unsigned char f5[8] = { 0,  0,  0, -1, -1, -1, -1, -1 };
		unsigned char f6[8] = { 0,  0, -1, -1, -1, -1, -1, -1 };
		unsigned char f7[8] = { 0, -1, -1, -1, -1, -1, -1, -1 };

		createChar(1, f1);
		createChar(2, f2);
		createChar(3, f3);
		createChar(4, f4);
		createChar(5, f5);
		createChar(6, f6);
		createChar(7, f7);
	}
}
void LCD_I2C::custom_set_3() {
	if (_cur_custom_set != 3) {
		_cur_custom_set = 3;
		customClear();

		unsigned char f1[8] = {  0,  0,  0,  0,  0,  0,  0, -1 };
		unsigned char f2[8] = {  0,  0,  0,  0,  0,  0, -1,  0 };
		unsigned char f3[8] = {  0,  0,  0,  0,  0, -1,  0,  0 };
		unsigned char f4[8] = {  0,  0,  0,  0, -1,  0,  0,  0 };
		unsigned char f5[8] = {  0,  0,  0, -1,  0,  0,  0,  0 };
		unsigned char f6[8] = {  0,  0, -1,  0,  0,  0,  0,  0 };
		unsigned char f7[8] = {  0, -1,  0,  0,  0,  0,  0,  0 };
		unsigned char f8[8] = { -1,  0,  0,  0,  0,  0,  0,  0 };

		createChar(0, f1);
		createChar(1, f2);
		createChar(2, f3);
		createChar(3, f4);
		createChar(4, f5);
		createChar(5, f6);
		createChar(6, f7);
		createChar(7, f8);
	}
}
