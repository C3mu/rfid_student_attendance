#ifndef easydislay_h
#define easydislay_h
#include <Wire.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
class easydislay {
  private :
    String do1, do2;
    unsigned int ld1, ld2;
    unsigned long  timed1,timed2;
  public:
    easydislay(void);
    void begin(){
      lcd.begin();
      lcd.backlight();
      lcd.noAutoscroll();
    }
    void checkdislay() {
      String rd1, rd2;
      if (do1.length() > 16) {
        if ( (unsigned long) (millis() - timed1) > 300 ) {
          ld1 += 1;
          timed1 = millis();
        }
        if (ld1 == do1.length() - 15)ld1 = 1;
        for (unsigned int a = ld1; a <= ld1 + 16; a++) {
          rd1 += do1[a];
        }
      }
      if (do1.length() <= 16) {
        rd1 = do1;
        for (int i = do1.length(); i <= 16; i++) {
          rd1 += " ";
        }
      }
      if (do2.length() > 16) {
        if ( (unsigned long) (millis() - timed2) > 300 ) {
          ld2 += 1;
          timed2 = millis();
        }
        if (ld2 == do2.length() - 15)ld2 = 1;
        for (unsigned int a = ld2; a <= ld2 + 16; a++) {
          rd2 += do2[a];
        }
      }
      if (do2.length() <= 16) {
        rd2 = do2;
        for (int i = do2.length(); i <= 16; i++) {
          rd2 += " ";
        }
      }
      lcd.setCursor(0, 0);
      lcd.print(rd1);
      lcd.setCursor(0, 1);
      lcd.print(rd2);

    }
    void show(String d1, String d2) {
      do1 = d1;
      do2 = d2;
      ld1 = ld2 = 0;
      //news = true;
    }
}


#endif

