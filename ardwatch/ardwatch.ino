/*
Arduino-based watch!
*/
#include <avr/wdt.h>
#include <MeetAndroid.h>
 #include "Wire.h"
 #include "SeeedOLED.h"
 #include <Time.h>
 #include "numbers.h"
 
 MeetAndroid meetAndroid;
 static const int BLANK_INTERVAL = 10000;
 static unsigned long blankCounter = 0;
void setup() 
{ 
  // Set the LED to indicate we're initializing
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  
  // temporarily disable the watchdog
  wdt_disable();
  
  // Commence system initialization
  Wire.begin();
  SeeedOled.init();
  SeeedOled.sendCommand(SeeedOLED_Display_Off_Cmd);
  
  Serial.begin(38400); //Set BluetoothFrame BaudRate to default baud rate 38400
  delay(1000);
  setupBlueToothConnection();    
  
  SeeedOled.clearDisplay();
  SeeedOled.sendCommand(SeeedOLED_Display_On_Cmd);
  SeeedOled.setNormalDisplay();
  SeeedOled.setHorizontalMode();
  SeeedOled.setTextXY(0,0);
  SeeedOled.sendCommand(SeeedOLED_Display_On_Cmd);
  
  //SeeedOled.putString("Hello World!");
  setTime(0, 0, 0, 1, 1, 2012);
  meetAndroid.registerFunction(setArdTime, 't');
  meetAndroid.registerFunction(setText, 'x');
  blankCounter = millis();
  
  // Set the watchdog timer in case we crash
  wdt_enable(WDTO_120MS);
  
  // enable pullup on the wake button so we can read it
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  MCUCR &= ~_BV(PUD);
  
  // indicate initialization done
  digitalWrite(8, LOW);
} 
 
void loop() 
{ 
  // walk the dog
  wdt_reset();
  
  // check for updates from the outside world
  meetAndroid.receive();
  
  // handle the inside world
  handleClockTasks();
} 

// TODO: allow set using formatted string?
void setArdTime(byte flag, byte numOfValues) {
  setTime(meetAndroid.getLong());
}

// We got a text!
void setText(byte flag, byte numOfValues) {
  // wake up so the user sees the text message
  if (blankCounter == 0) {
    wakeClock();
  }
  
  for (int jj=0; jj < 2; ++jj) {
    wdt_reset();
    SeeedOled.setTextXY(jj+1,0);
    // Clear the text area
    for (int ii=0; ii < 16; ++ii) { SeeedOled.putChar(' '); }
  }
  SeeedOled.setTextXY(1,0);
  
  int length = meetAndroid.stringLength();
  
  // define an array with the appropriate size which will store the string
  int data;
  
  // tell MeetAndroid to put the string into your prepared array
  while ((data = meetAndroid.getChar()) > 0) {
    wdt_reset();
    SeeedOled.putChar(data);
  }
}

void handleClockTasks() {
  // If we're awake, see if it's time to sleep
  if (blankCounter > 0) {
    if (millis() - blankCounter >= BLANK_INTERVAL) {
      sleepClock();
    }
  }
  
  // Extend the wake if the button is down
  if (digitalRead(2) == LOW) {
    digitalWrite(8, HIGH);
    wakeClock();
  } else {
    digitalWrite(8, LOW);
  }
  
  // If we're awake, display the time
  static time_t last_display = 0;
  if (blankCounter && last_display != now()) {
    char tbuf[32];
    last_display = now();
    
    SeeedOled.setTextXY(3,0);
    snprintf(tbuf, 12, "%02d:%02d:%02d", hour(), minute(), second());
    SeeedOled.putString(tbuf);
    
    SeeedOled.setTextXY(5,0);
    snprintf(tbuf, 12, "%02d/%02d/%04d", month(), day(), year());
    SeeedOled.putString(tbuf);    
  }
}

void sleepClock() {
   wdt_disable();
  SeeedOled.sendCommand(SeeedOLED_Display_Off_Cmd);
  blankCounter = 0;
}

void wakeClock() {
  SeeedOled.sendCommand(SeeedOLED_Display_On_Cmd);
  blankCounter = millis();
  wdt_enable(WDTO_120MS);
}

void setupBlueToothConnection()
{
 
    sendBlueToothCommand("+STWMOD=0");
    sendBlueToothCommand("+STNA=ArdWatch_00001");
    sendBlueToothCommand("+STAUTO=0");
    sendBlueToothCommand("+STOAUT=1");
    sendBlueToothCommand("+STPIN=0000");
    delay(3000); // This delay is required.
    sendBlueToothCommand("+INQ=1");
    delay(3000); // This delay is required.
 
}
 
 
//Send the command to Bluetooth Frame
void sendBlueToothCommand(char command[])
{
    Serial.println();
    delay(200);
    Serial.print(command);
    delay(200);
    Serial.println();
    delay(1000);   
    Serial.flush();
}
