#include <Wire.h> 

#include "RTClib.h"
#include "pits_burner.h"
#include "pits_display.h"

RTC_DS3231 rtc;
Scheduler scheduler;

void setup() {
    
  //init serial
  Serial.begin(115200);
  Serial.println("Serial initialized...");

  //init i2c
  Wire.begin();

  //init RTC with time from PC on compilation
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.println("RTC initialized...");

  // Set timer 3 pins 3,11 to 31 Hz (31250/1024 = 31) for MOSFET control
  setPwmFrequency(3, 1024);
  setPwmFrequency(11, 1024);
   
  burner.init(A0, A1, A3, A2, 3, 11); //TBoiler, TExhaust, FlameSensor, TFeeder, Fan, Feeder
  display.init();

}

void loop() {

  // send data only when you receive data:
  if (Serial.available() > 0) {
      String incomingCommand = Serial.readString();
      if (incomingCommand.startsWith("TEMP=")) {                       //TEMP - set required boiler temperature
        incomingCommand.remove(0,4);
        burner.setRequiredTemp(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("FEED_HEAT=")) {             //FEED_HEAT - set feed time (seconds) in heat mode
      }
      else if (incomingCommand.startsWith("FEED_DELAY_HEAT=")) {       //FEED_DELAY_HEAT - set feed delay (seconds) in heat mode
      }
      else if (incomingCommand.startsWith("FEED_IDLE=")) {             //FEED_IDLE - set feed time (seconds) in idle mode
      }
      else if (incomingCommand.startsWith("FEED_DELAY_IDLE=")) {       //FEED_DELAY_IDLE - set feed delay (seconds) in idle mode
      }
      else if (incomingCommand.startsWith("FAN_HEAT=")) {              //FAN_HEAT - set fan power (%) in heat mode
      }
      else if (incomingCommand.startsWith("FAN_IDLE=")) {              //FAN_IDLE - set fan power (%) in idle mode
      }
      else if (incomingCommand.startsWith("BOILER=")) {                //BOILER - overwrite current boiler temberature readings (service mode)
        incomingCommand.remove(0,4);
        burner.setCurrentTemp(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("EXHAUST=")) {               //EXHAUST - overwrite current exhaust temberature readings (service mode)
        incomingCommand.remove(0,4);
        burner.setExhaustTemp(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("FLAME=")) {                 //FLAME - overwrite flame detection flag readings (service mode)
        incomingCommand.remove(0,4);
        burner.setFlame(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("FEED=")) {                  //FEED - overwrite feeder On=1 or Off=0
        incomingCommand.remove(0,4);
        burner.setFeed((bool)incomingCommand.toInt());
        burner.onFeed();
      }
      else if (incomingCommand.startsWith("STOP")) {                  //set mode: STOP
        burner.setCurrentMode(MODE_STOP);
      }
      else if (incomingCommand.startsWith("IGNITE")) {                //set mode: IGNITE
        burner.setCurrentMode(MODE_IGNITION);
      }
      else if (incomingCommand.startsWith("HEAT")) {                  //set mode: HEAT
        burner.setCurrentMode(MODE_HEAT);
      }
      else if (incomingCommand.startsWith("IDLE")) {                  //mode overwrite: IDLE
        burner.setCurrentMode(MODE_IDLE);
      }
      else if (incomingCommand.startsWith("ALARM")) {                 //mode overwrite: ALARM
        burner.setCurrentMode(MODE_ALARM);
      }
      else if (incomingCommand.startsWith("CLEAN")) {                 //mode overwrite: CLEANING
        burner.setCurrentMode(MODE_CLEANING);
      }
      else {
        Serial.print("Unrecognized command: " + incomingCommand);
      }
  }
  
  scheduler.execute();
}

/**
 * Divides a given PWM pin frequency by a divisor.
 * 
 * The resulting frequency is equal to the base frequency divided by
 * the given divisor:
 *   - Base frequencies:
 *      o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
 *      o The base frequency for pins 5 and 6 is 62500 Hz.
 *   - Divisors:
 *      o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
 *        256, and 1024.
 *      o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
 *        128, 256, and 1024.
 * 
 * PWM frequencies are tied together in pairs of pins. If one in a
 * pair is changed, the other is also changed to match:
 *   - Pins 5 and 6 are paired on timer0
 *   - Pins 9 and 10 are paired on timer1
 *   - Pins 3 and 11 are paired on timer2
 * 
 * Note that this function will have side effects on anything else
 * that uses timers:
 *   - Changes on pins 3, 5, 6, or 11 may cause the delay() and
 *     millis() functions to stop working. Other timing-related
 *     functions may also be affected.
 *   - Changes on pins 9 or 10 will cause the Servo library to function
 *     incorrectly.
 * 
 * Thanks to macegr of the Arduino forums for his documentation of the
 * PWM frequency divisors. His post can be viewed at:
 *   http://forum.arduino.cc/index.php?topic=16612#msg121031
 */
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

