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

              // TBoiler, TExhaust, FlameSensor, TFeeder, Fan, Feeder, Igniter
  burner.init(   A0,      A1,       A4,          A5,      3,   11,     5       ); 
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
      else if (incomingCommand.startsWith("BOILER=")) {                //BOILER - overwrite current boiler temberature readings (service mode)
        incomingCommand.remove(0,7);
        burner.setCurrentTemp(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("EXHAUST=")) {               //EXHAUST - overwrite current exhaust temberature readings (service mode)
        incomingCommand.remove(0,8);
        burner.setExhaustTemp(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("FLAME=")) {                 //FLAME - overwrite flame detection flag readings (service mode)
        incomingCommand.remove(0,5);
        burner.setFlame(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("FEED=")) {                  //FEED - overwrite feeder to speed 0-100
        incomingCommand.remove(0,5);
        burner.setFeed(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("FAN=")) {                  //FEED - overwrite fan to speed 0-100
        incomingCommand.remove(0,4);
        burner.setFan(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("IGNITION=")) {             //IGNITION - overwrite igntitor status (WARNING! ignitor can not be ON more than 5 seconds)
        incomingCommand.remove(0,9);
        burner.setIgnition((bool)incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("MANUAL") || 
               incomingCommand.startsWith("STOP")) {                  //set mode: MANUAL
        burner.setCurrentMode(MODE_MANUAL);
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



