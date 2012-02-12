#include <avr/sleep.h>
#include <Wire.h>
#include <SeeedOLED.h>
#include "Time.h"

void setup()
{
  Wire.begin();	
  SeeedOled.init();  //initialze SEEED OLED display
  DDRB|=0x21;        //digital pin 8, LED glow indicates Film properly Connected .
  PORTB |= 0x21;
 
  SeeedOled.clearDisplay();           //clear the screen and set start position to top left corner
  SeeedOled.setNormalDisplay();       //Set display to Normal mode
  SeeedOled.setHorizontalMode();      //Set addressing mode to Horizontal Mode
  SeeedOled.putString("Initializing");  //Print String (ASCII 32 - 126 )
 
  initTime();
  setTime(1);
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);
}
 
void loop()
{
  unsigned long lastMillis = 0;
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sei();
  sleep_enable();
  while (lastMillis == myMillis()) {
    sleep_cpu();
  }
  sleep_disable();
  lastMillis = myMillis();
  Serial.println(myMillis());
  updateTime();
  delay(10);
}

void updateTime() {
  char buf[32];
  static unsigned long start = 0;
  
  sprintf(buf, "%02d:%02d:%02d %.2s", hourFormat12(), minute(), second(), isAM() ? "AM" : "PM");
  SeeedOled.setTextXY(3, 0);
  SeeedOled.putString(buf);
  
  sprintf(buf, "%.3s %02d, %04d", monthShortStr(month()), day(), year());
  SeeedOled.setTextXY(4, 0);
  SeeedOled.putString(buf);

  sprintf(buf, "%s", dayStr(weekday()));
  SeeedOled.setTextXY(5, 2);
  SeeedOled.putString(buf);
  if (start == 0) {
    if (second() % 10 == 0) {
      start = myMillis();
      digitalWrite(5, HIGH);
    }
  } else {
    if (myMillis() - start >= 250) {
      digitalWrite(5, LOW);
    }
    if (myMillis() - start >= 1000) {
      start = 0;
    }
  }
}
