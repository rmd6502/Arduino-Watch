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
// #include "numbers.h"
 
 MeetAndroid meetAndroid;
 static const int BLANK_INTERVAL_MS = 20000;
 static const int TEMP_INTERVAL_S = 60;
 static unsigned long blankCounter = 0;
 static volatile uint8_t int0_awake = 0;
 static volatile uint8_t vector = 0;
void setup() 
{ 
  // Set the LED to indicate we're initializing
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  
  Serial.begin(38400); //Set BluetoothFrame BaudRate to default baud rate 38400
  
  // temporarily disable the watchdog
  wdt_disable();
  
  // Commence system initialization
  Wire.begin();
  SeeedOled.init();
  SeeedOled.sendCommand(SeeedOLED_Display_Off_Cmd);
  
  delay(300);
    
  setupBlueToothConnection();    
   
  SeeedOled.clearDisplay();
  SeeedOled.sendCommand(SeeedOLED_Display_On_Cmd);
  SeeedOled.setNormalDisplay();
  SeeedOled.setHorizontalMode();
  SeeedOled.setTextXY(0,0);
  SeeedOled.sendCommand(SeeedOLED_Display_On_Cmd);
    
  //SeeedOled.putString("Hello World!");
  setTime(0, 0, 0, 1, 1, 2012);
  meetAndroid.flush();
  meetAndroid.registerFunction(setArdTime, 't');
  meetAndroid.registerFunction(setText, 'x');
  meetAndroid.registerFunction(sendBattery, 'b');
  meetAndroid.registerFunction(sendTemp, 'm');
  
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
  
  // indicate initialization done
  meetAndroid.send("init done");
  digitalWrite(8, LOW);
} 
 
void loop() 
{ 
  // walk the dog
  wdt_reset();
  
  // check for updates from the outside world
  meetAndroid.receive();
  
  /* wdt_reset */;
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
  
  SeeedOled.setTextXY(1,0);
  // Clear the text area
  for (int ii=0; ii < 32; ++ii) { SeeedOled.putChar(' ');  wdt_reset(); }
  SeeedOled.setTextXY(1,0);
  
  int length = meetAndroid.stringLength();
  
  // define an array with the appropriate size which will store the string
  char data[length];
  
  // tell MeetAndroid to put the string into your prepared array
  meetAndroid.getString(data);
  //wdt_reset();
  SeeedOled.putString(data);
  //wdt_reset();
}

void handleClockTasks() {
  static long tempReading = 0;
  //static time_t lastTemp = 0;
  
  // If we're awake, see if it's time to sleep
  if (blankCounter > 0) {
    if (millis() - blankCounter >= BLANK_INTERVAL_MS) {
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
  
  if (now() - lastTemp >= TEMP_INTERVAL_S) {
    tempReading = readTemp() / 1000;
    lastTemp = now();
  }
  
  // If we're awake, display the time
  //static time_t last_display = 0;
  if (blankCounter && last_display != now()) {
    char tbuf[32];
    last_display = now();
    
    meetAndroid.send("tick");
    
    SeeedOled.setTextXY(3,0);
    snprintf(tbuf, 12, "%02d:%02d:%02d", hour(), minute(), second());
    SeeedOled.putString(tbuf);
    
    SeeedOled.setTextXY(5,0);
    snprintf(tbuf, 12, "%02d/%02d/%04d", month(), day(), year());
    SeeedOled.putString(tbuf);
    
    SeeedOled.setTextXY(7,0);
    snprintf(tbuf, 16, "Temp: %3dC %3dF", tempReading, tempReading * 9/5 + 32);
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
  wdt_reset();
  wdt_disable();
  
  meetAndroid.send("going to sleep");
  delay(100);
    
  SeeedOled.sendCommand(SeeedOLED_Display_Off_Cmd);
  blankCounter = 0;
  
  //set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
  //sei();
  //sleep_enable();
  //sleep_cpu();
  //sleep_disable();
}

void wakeClock() {
  wdt_disable();
  SeeedOled.sendCommand(SeeedOLED_Display_On_Cmd);
  blankCounter = millis();
  meetAndroid.send("waking up");
  delay(100);
  //wdt_enable(WDTO_500MS);
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

// Interrupt handlers
ISR(INT0_vect) {
  int0_awake = 1;
}

ISR(BADISR_vect) {
  vector = 1;
}


void sendBattery(byte flag, byte numOfValues) {
  long temp = readVcc();
  meetAndroid.send(temp);

}

long readVcc() {
  long result;
  
  // Enable the ADC
  PRR &= ~_BV(0);
  ADCSRA |= _BV(ADEN);
  
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC)) wdt_reset();
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  
  // disable the ADC
  ADCSRA &= !_BV(ADEN);
  PRR |= _BV(0);
  return result;
}

void sendTemp(byte flag, byte numOfValues) {
  long temp = readTemp();
  meetAndroid.send(temp);
}

long readTemp() {
  long result;
  
  // Enable the ADC
  PRR &= ~_BV(0);
  ADCSRA |= _BV(ADEN);
  
  // Read temperature sensor against 1.1V reference
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC)) wdt_reset();
  result = ADCL;
  result |= ADCH<<8;
  result = (result - 125) * 1075;
  
  // disable the ADC
  ADCSRA &= !_BV(ADEN);
  PRR |= _BV(0);
  
  return result;
}
