#include <LCD_I2C.h>

LCD_I2C lcd(0x27, 16, 2);

byte l[16] = {};

void setup() {
  lcd.begin();
  l[15] = (sin(0.5)+1)*8;
}

float s = 0.5;

void loop() {
  s += 0.5;
  float x = sin(s)+1;
  for (int i = 0; i < 15; i += 1) {
    l[i] = l[i + 1];
  }
  l[15] = x * 8;

  // Make plot at position 0x0 size 16x2 with contens at "l" array
  lcd.makePlot(0, 0, 16, 2, l);
  delay(400);
}
