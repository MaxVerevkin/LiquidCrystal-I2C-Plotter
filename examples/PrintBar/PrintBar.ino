#include <LCD_I2C.h>

LCD_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.begin();
  // Print empty bar at position 0x1 with length 16
  // (so max value is 16 * 5 = 80)
  // Use BAR_BORDERS to have borders
  // And use BAR_NOBORDERS not to have
  lcd.printBar(0, 1, 16, 0, BAR_BORDERS);
  delay(100);
}

byte num = 0;
int inc = 1; // Go up

void loop() {
  num += inc;
  if (num == 80) {
    inc = -1; // Go down
  } else if (num == 0) {
    inc = 1; // Go up
  }
  // Print bar
  if (inc > 0) {
    lcd.printBar(0, 1, 16, num); // BAR_BORDERS is default
  } else {
    lcd.printBar(0, 1, 16, num, BAR_NOBORDERS);
  }
}