#include <LCD_I2C.h>

LCD_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.begin();
}

void loop() {
  lcd.clear();
  lcd.print("Demo custom sets");
  delay(2000);
  lcd.clear();
  
  lcd.print("custom_set_0");
  lcd.custom_set_0();
  lcd.setCursor(0, 1);
  for (int i = 0; i < 8; i += 1) {
    lcd.write(i);
    lcd.print(' ');
  }
  delay(5000);
  lcd.clear();
  
  lcd.print("custom_set_1");
  lcd.custom_set_1();
  lcd.setCursor(0, 1);
  for (int i = 0; i < 8; i += 1) {
    lcd.write(i);
    lcd.print(' ');
  }
  delay(5000);
  lcd.clear();
  
  lcd.print("custom_set_2");
  lcd.custom_set_2();
  lcd.setCursor(0, 1);
  for (int i = 0; i < 8; i += 1) {
    lcd.write(i);
    lcd.print(' ');
  }
  delay(5000);
  lcd.clear();
  
  lcd.print("custom_set_3");
  lcd.custom_set_3();
  lcd.setCursor(0, 1);
  for (int i = 0; i < 8; i += 1) {
    lcd.write(i);
    lcd.print(' ');
  }
  delay(5000);
  lcd.clear();
  
  lcd.print("custom_set_4");
  lcd.custom_set_4();
  lcd.setCursor(0, 1);
  for (int i = 0; i < 8; i += 1) {
    lcd.write(i);
    lcd.print(' ');
  }
  delay(5000);
  lcd.clear();
  
  lcd.customClear();
}