#include <Wire.h> 

#include "RTClib.h"
#include "pits_burner.h"
#include "pits_display.h"

RTC_DS3231 rtc;
Scheduler scheduler;

void setup() {
    
  //init serial
  Serial.begin(57600);
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

  burner.init(A5, A6, A7, A4, 6, 5);
  display.init();

}

void loop() {

  // send data only when you receive data:
  if (Serial.available() > 0) {
      String incomingCommand = Serial.readString();
      if (incomingCommand.startsWith("SRT=")) {            //set required boiler temperature
        incomingCommand.remove(0,4);
        burner.setRequiredTemp(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("FTH=")) {       //FTH - set feed time (seconds) in heat mode
      }
      else if (incomingCommand.startsWith("FDH=")) {       //FDH - set feed delay (seconds) in heat mode
      }
      else if (incomingCommand.startsWith("FTI=")) {       //FTI - set feed time (seconds) in idle mode
      }
      else if (incomingCommand.startsWith("FDI=")) {       //FDI - set feed delay (seconds) in idle mode
      }
      else if (incomingCommand.startsWith("FPH=")) {       //FPH - set fan power (%) in heat mode
      }
      else if (incomingCommand.startsWith("FPI=")) {       //FPI - set fan power (%) in idle mode
      }
      else if (incomingCommand.startsWith("SBT=")) {       //SBT - overwrite current boiler temberature readings (service mode)
        incomingCommand.remove(0,4);
        burner.setCurrentTemp(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("SET=")) {       //SET - overwrite current exhaust temberature readings (service mode)
        incomingCommand.remove(0,4);
        burner.setExhaustTemp(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("SFF=")) {       //SFF - overwrite flame detection flag readings (service mode)
        incomingCommand.remove(0,4);
        burner.setFlame(incomingCommand.toInt());
      }
      else if (incomingCommand.startsWith("FDR=")) {       //FDR - overwrite feeder On or Off
        incomingCommand.remove(0,4);
        burner.setFeed((bool)incomingCommand.toInt());
        burner.onFeed();
      }
      else if (incomingCommand.startsWith("STOP")) {       //mode: stop
        burner.setCurrentMode(MODE_STOP);
      }
      else if (incomingCommand.startsWith("IGNITE")) {     //mode: ignite
        burner.setCurrentMode(MODE_IGNITION);
      }
      else if (incomingCommand.startsWith("HEAT")) {       //mode: heat
        burner.setCurrentMode(MODE_HEAT);
      }
      else if (incomingCommand.startsWith("IDLE")) {       //mode overwrite: idle
        burner.setCurrentMode(MODE_IDLE);
      }
      else if (incomingCommand.startsWith("ALARM")) {       //mode overwrite: alarm
        burner.setCurrentMode(MODE_ALARM);
      }
      else if (incomingCommand.startsWith("CLEAN")) {       //mode overwrite: cleaning
        burner.setCurrentMode(MODE_CLEANING);
      }
      else {
        Serial.print("Unrecognized command: " + incomingCommand);
      }
  }

  
  scheduler.execute();



  //delay(500);
}


