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
 static const unsigned int BLANK_INTERVAL_MS = 10000;
 static const time_t TEMP_INTERVAL_S = 60;
 static unsigned long blankCounter = 0;
 static volatile uint8_t int0_awake = 0;
 static volatile uint8_t pcint2_awake = 0;
 
#define WATCHDOG_INTERVAL (WDTO_500MS)

void setup() 
{ 
  // TODO: detect brownout and keep processor shut down with BOD disabled
  // we'll need to let it wake when charge power is applied, hook !CHG to INT1
  
  // Set the LED to indicate we're initializing
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  
  Serial.begin(38400); //Set BluetoothFrame BaudRate to default baud rate 38400
  
  // temporarily disable the watchdog
  wdt_disable();
  
  // Commence system initialization
  Wire.begin();
  SeeedOled.init();
  digitalWrite(8, HIGH);
  
  delay(100);
  SeeedOled.sendCommand(SeeedOLED_Display_Off_Cmd);
  digitalWrite(8, LOW);
  
  setupBlueToothConnection();   
  digitalWrite(8, HIGH);
   
  SeeedOled.clearDisplay();
//  SeeedOled.sendCommand(SeeedOLED_Display_On_Cmd);
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
  
  // Set interrupt on the button press
  cli();
  // interrupt on falling edge
  EICRA = 1;
  // prevent false alarms
  EIFR |= 3;
  // Enable interrupt on wake button
  EIMSK |= _BV(INT0);
  
  // set up pcint2 for changes to rxd
  PCIFR |= _BV(PCIF2);
  PCMSK2 |= _BV(PCINT16) | _BV(PCINT18);
  sei();
  
  // indicate initialization done    
  //meetAndroid.send("init done");
  wdt_enable(WATCHDOG_INTERVAL);
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
  long newTime = meetAndroid.getLong();
  if (newTime > 0) {
    setTime(newTime);
  }
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
  if (length > 32) {
    meetAndroid.send("truncating large string");
    length = 32;
  }
  
  // define an array with the appropriate size which will store the string
  int data;
  
  // tell MeetAndroid to put the string into your prepared array
  while (length-- > 0 && (data = meetAndroid.getChar()) > 0) {
    wdt_reset();
    SeeedOled.putChar(data);
  }
}

void handleClockTasks() {
  static long tempReading = 0;
  static time_t lastTemp = 0;
  
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
  static time_t last_display = 0;
  if (blankCounter && last_display != now()) {
    char tbuf[32];
    last_display = now();
    
    SeeedOled.setTextXY(3,0);
    snprintf(tbuf, 12, "%02d:%02d:%02d", hour(), minute(), second());
    SeeedOled.putString(tbuf);
    
    SeeedOled.setTextXY(5,0);
    snprintf(tbuf, 16, "%02d/%02d/%04d %.3s", month(), day(), year(), dayShortStr(day()));
    SeeedOled.putString(tbuf);
    
    SeeedOled.setTextXY(7,0);
    snprintf(tbuf, 16, "Temp: %3ldC %3ldF", tempReading, tempReading * 9/5 + 32);
    SeeedOled.putString(tbuf);
  }
  
  if (int0_awake) {
    int0_awake = 0;
    meetAndroid.send("wakened by int0");
  }
  if (pcint2_awake) {
    pcint2_awake = 0;
    meetAndroid.send("wakened by pcint2");
  }
}

void sleepClock() {
  // Don't want the dog barking while we're napping
  wdt_reset();
  wdt_disable();
  
  meetAndroid.send(">sleep");
  delay(100);
    
  SeeedOled.sendCommand(SeeedOLED_Display_Off_Cmd);
  blankCounter = 0;
  
//    #define SLEEP_MODE_IDLE         (0)
//    #define SLEEP_MODE_ADC          _BV(SM0)
//    #define SLEEP_MODE_PWR_DOWN     _BV(SM1)
//    #define SLEEP_MODE_PWR_SAVE     (_BV(SM0) | _BV(SM1))
//    #define SLEEP_MODE_STANDBY      (_BV(SM1) | _BV(SM2))
//    #define SLEEP_MODE_EXT_STANDBY  (_BV(SM0) | _BV(SM1) | _BV(SM2))
  
  cli();
  PCIFR |= _BV(PCIF2);
  PCICR |= _BV(PCIE2);
  
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sei();
  sleep_enable();
  while (int0_awake == 0 && pcint2_awake == 0) {
    sleep_cpu();
  }
  sleep_disable();
  
  PCICR &= ~_BV(PCIE2);
  meetAndroid.send("Wake");
}

void wakeClock() {
  wdt_disable();
  sei();
  SeeedOled.sendCommand(SeeedOLED_Display_On_Cmd);
  blankCounter = millis();
  delay(100);
  wdt_enable(WATCHDOG_INTERVAL);
}

void setupBlueToothConnection()
{
    sendBlueToothCommand((char *)"+STWMOD=0");
    sendBlueToothCommand((char *)"+STNA=ArdWatch_00001");
    sendBlueToothCommand((char *)"+STAUTO=0");
    sendBlueToothCommand((char *)"+STOAUT=1");
    sendBlueToothCommand((char *)"+STPIN=0000");
    delay(3000); // This delay is required.
    sendBlueToothCommand((char *)"+INQ=1");
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

ISR(PCINT2_vect) {
  pcint2_awake = 1;
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
  ADCSRA &= ~_BV(ADEN);
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
  ADCSRA &= ~_BV(ADEN);
  PRR |= _BV(0);
  
  return result;
}

