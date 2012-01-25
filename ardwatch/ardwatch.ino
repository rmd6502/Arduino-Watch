/*
Arduino-based watch!
*/
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <MeetAndroid.h>
 #include "Wire.h"
 #include "SeeedOLED.h"
 #include <Time.h>
 #include "numbers.h"
 
 MeetAndroid meetAndroid;
 static const int BLANK_INTERVAL = 10000;
 static unsigned long blankCounter = 0;
 static volatile uint8_t int0_awake = 0;
 static volatile uint8_t vector = 0;
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
  
  // enable pullup on the wake button so we can read it
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  MCUCR &= ~_BV(PUD);
  
  //cli();
  // interrupt on falling edge
  //EICRA = 2;
  // prevent false alarms
  //PCICR = 0;
  //EIFR |= 3;
  // Enable interrupt on wake button
  //EIMSK |= _BV(INT0);
  //sei();

  // Set the watchdog timer in case we crash
  //wdt_enable(WDTO_120MS);
  
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
  
  meetAndroid.send("blankCounter "); 
  meetAndroid.send(blankCounter);
  meetAndroid.sendln();
  
  // wake up so the user sees the text message
  if (blankCounter == 0) {
    wakeClock();
  }
  
  SeeedOled.setTextXY(1,0);
  // Clear the text area
  SeeedOled.putString("                                ");
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
  if (blankCounter) {
    char tbuf[12];
    
    SeeedOled.setTextXY(3,0);
    snprintf(tbuf, 12, "%02d:%02d:%02d", hour(), minute(), second());
    SeeedOled.putString(tbuf);
    
    SeeedOled.setTextXY(5,0);
    snprintf(tbuf, 12, "%02d/%02d/%04d", month(), day(), year());
    SeeedOled.putString(tbuf);    
  }
  
  if (int0_awake) {
    int0_awake = 0;
    meetAndroid.send("wakened by int0");
  }
  if (vector) {
    vector = 0;
    meetAndroid.send("wakened by bad vector");
  }
}

void sleepClock() {
  // Don't want the dog barking while we're napping
  //wdt_reset();
  //wdt_disable();
  //sei();
  
  meetAndroid.send("going to sleep");
  delay(500);
  
  SeeedOled.sendCommand(SeeedOLED_Display_Off_Cmd);
  blankCounter = 0;
  
  //set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
  //sei();
  //sleep_enable();
  //sleep_cpu();
  //sleep_disable();
}

void wakeClock() {
  SeeedOled.sendCommand(SeeedOLED_Display_On_Cmd);
  blankCounter = millis();
  meetAndroid.send("waking up");
  delay(500);
  //wdt_enable(WDTO_120MS);
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

// Interrupt handlers
ISR(INT0_vect) {
  int0_awake = 1;
  sei();
}

ISR(BADISR_vect) {
  vector = 1;
  sei();
}

