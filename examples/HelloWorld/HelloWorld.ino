#include <LCD_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LCD_I2C lcd(0x27, 16, 2);

void setup()
{
	// initialize the LCD
	lcd.begin();

	// Print the message
	lcd.print("Hello, world!");
}

void loop()
{
	// Do nothing here...
}
