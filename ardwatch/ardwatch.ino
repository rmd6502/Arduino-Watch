                  /*
BluetoothFrame Demo Code
2011 Copyright (c) Seeed Technology Inc.  All right reserved.
 
Author: Visweswara R
 
This demo code is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
 
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
For more details about the product please check http://www.seeedstudio.com/depot/
 
*/
 
 
/* 1.Upload this sketch to Seeeduino Film
   2.Power Off Seeeduino Film
   3.Connect Bluetooth Frame
   4.Connect Battery
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
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  wdt_disable();
    Wire.begin();
    SeeedOled.init();
    
    Serial.begin(38400); //Set BluetoothFrame BaudRate to default baud rate 38400
    delay(1000);
    setupBlueToothConnection();    
    
  SeeedOled.clearDisplay();
  SeeedOled.setNormalDisplay();
  SeeedOled.setHorizontalMode();
  SeeedOled.setTextXY(0,0);
  
  //SeeedOled.putString("Hello World!");
  setTime(0, 0, 0, 1, 1, 2012);
  meetAndroid.registerFunction(setArdTime, 't');
  meetAndroid.registerFunction(setText, 'x');
  blankCounter = millis();
  wdt_enable(WDTO_120MS);
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  MCUCR &= ~_BV(PUD);
  digitalWrite(8, LOW);
} 
 
void loop() 
{ 
  wdt_reset();
  meetAndroid.receive();
  handleClockTasks();
} 
 
void setArdTime(byte flag, byte numOfValues) {
  setTime(meetAndroid.getLong());
}

void setText(byte flag, byte numOfValues) {
  meetAndroid.send("blankCounter "); 
  meetAndroid.send(blankCounter);
  meetAndroid.sendln();
  if (blankCounter == 0) {
    wakeClock();
  }
  SeeedOled.setTextXY(1,0);
  
  int length = meetAndroid.stringLength();
  meetAndroid.send("length "); 
  meetAndroid.send(length);
  meetAndroid.sendln();
  
  // define an array with the appropriate size which will store the string
  char data[length+1];
  
  // tell MeetAndroid to put the string into your prepared array
  meetAndroid.getString(data);
  data[31] = 0;
  SeeedOled.putString(data);
}

void handleClockTasks() {
  if (blankCounter > 0) {
    if (millis() - blankCounter >= BLANK_INTERVAL) {
      sleepClock();
    }
  }
  if (digitalRead(2) == LOW) {
    digitalWrite(8, HIGH);
    wakeClock();
  } else {
    digitalWrite(8, LOW);
  }
  if (blankCounter) {
    char tbuf[12];
    
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
    sendBlueToothCommand("+STNA=SeeeduinoBluetooth");
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
