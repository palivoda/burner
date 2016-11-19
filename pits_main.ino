#include <Wire.h> 

#include "RTClib.h"
#include "pits_burner.h"
#include "pits_display.h"

RTC_DS3231 rtc;
Scheduler scheduler;
/*
#include <SPI.h>
#include <Ethernet.h>
EthernetClient client;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static void onLog();
Task tLogger(30*TASK_SECOND, TASK_FOREVER, onLog, &scheduler, true);
*/
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

  /*
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Ethernet initialization failed, check DHCP...");
  }
  else {
    Serial.println("Ethernet initialized...");
    delay(1000);
  }
  */
              // TBoiler, TExhaust, FlameSensor, TFeeder, Fan, Feeder, Igniter, Alarm
  burner.init(   A8,      A9,       A12,         A13,      3,   9,     6,        4 ); 
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

/* 
   
 // http://www.open-electronics.org/how-send-data-from-arduino-to-google-docs-spreadsheet/
static void onLog() {
  Serial.print("OnLog - ");

  String data = "";
  data+="entry.1600238660=";
  data+=
      String(burner.getCurrentTemp()) + "-"  + 
      burner.getFeederTemp() + "-"  + 
      burner.getExhaustTemp() + "-"  + 
      burner.getFeedTime() + "-"  + 
      burner.getSecondsWithoutFlame() + "-"  + 
  data+="&submit=Submit";

  if (client.connected()) client.stop();
    
  if (client.connect("docs.google.com", 80)) {
    Serial.print("connected - ");
    Serial.println(data);

    //https://docs.google.com/forms/d/e/1FAIpQLScgCwW9Wc9DIz_8zGQKSCb2Tth16buEd9y6Jn18KwuFcVvHfw/formResponse?ifq&entry.1600238660=Hello%20World&submit=Submit%22
    client.print("POST /formResponse?formkey=");
    client.print("1FAIpQLScgCwW9Wc9DIz_8zGQKSCb2Tth16buEd9y6Jn18KwuFcVvHfw");
    client.println("&ifq HTTP/1.1");
    client.println("Host: spreadsheets.google.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println();
 
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}
*/

