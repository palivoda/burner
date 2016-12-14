/*
  pits_burner - Class for pellets burner control.
  Created by Rostislav Palivoda, October 15, 2016.
  GNU GPL3 Licence.
*/
#ifndef _PITSBURNER_H_
#define _PITSBURNER_H_

#include <Arduino.h>

//#define _BURNER_DEBUG_SERIAL_
//#define _BURNER_SET_DEBUG_SERIAL_

enum PitsBurnerMode {
  MODE_MANUAL = 1,
  MODE_IGNITION = 2,
  MODE_HEAT = 3,
  MODE_IDLE = 4,
  MODE_CLEANING = 5,
  MODE_ALARM = 6
};

enum PitsAlarmStatus {
  ALARM_OK,
  ALARM_TEMPDROP,
  ALARM_NOFLAME,
  ALARM_IGNITION_FAILED,
  ALARM_OVERHEAT,
  ALARM_MANUAL
};

class PitsBurner
{
  public:
    
    void init();
    void operate();

    //events, timers
    static void onFeed();
    static void onFan();
    static void onIgnite();
    void onAlarmOn();
    void onAlarmOff();
    
    //sensor readings and contol signals
    void setCurrentTemp(byte);  //to overwrite sensors value
    byte getCurrentTemp();
    void setExhaustTemp(byte);  //to overwrite sensors value
    byte getExhaustTemp();
    void setFeederTemp(byte);   //to overwrite sensors value
    byte getFeederTemp();

    void setFlame(byte);        //to overwrite sensors value
    byte getFlame();
    bool isFlame();
    
    void setFan(byte);
    byte getFan();
    bool isFan();

    void setFeed(byte);
    byte getFeed();
    bool isFeed();
    byte getFeedTime();
    
    void setIgnition(bool);
    bool isIgnition();

    byte getBattLevel();
    void setBattLevel(byte);
    
    byte getFuelLevel();
    void setFuelLevel(byte);
    
    bool isPump();
    void setPump(bool);
    
    unsigned int getSecondsWithoutFlame();
    void updateLastFlameStatus();
    void resetFlameTimer();

    PitsAlarmStatus getAlarmStatus();
    PitsBurnerMode getCurrentMode();
    bool setCurrentMode(PitsBurnerMode, PitsAlarmStatus);
    
  private:
    void _readSensors();
    void _switchMode();
    float _KTY81_210(float);
    float _LDR04(float);
    void _setAlarmStatus(PitsAlarmStatus);
    byte _HYSRF05();

    //physical pins connection
    const byte _pinTBoiler = A0;          //Analog in
    const byte _pinTExhaust = A1;         //Analog in
    const byte _pinFlameSensor = A2;      //Analog in
    const byte _pinTFeeder = A3;          //Analog in
    const byte _pinFan = A6;              //PWM out
    const byte _pinFeeder = A7;           //PWM out
    const byte _pinIgniter = 3;           //Digital out
    const byte _pinAlarm = 2;             //Digital out
    const byte _pinBattery = A5;          //Analog in
    const byte _pinFuelTrig = 7;          //Digital out
    const byte _pinFuelEcho = 8;          //Digital in
    const byte _pinPump = 9;              //Digital out

    //sensors read values
    byte _intCurrentTemp = 0;
    byte _intExhaustTemp = 0;
    byte _intFeederTemp = 0;
    byte _intFlame = 0;
    byte _intBattLevel = 0;  //TODO
    byte _intFuelLevel = 0;
    const short _msFuelTimout = 15000;     //// depends on max distance - 3000 µs = 50cm // 30000 µs = 5 m
    
    //object internal states
    unsigned int _intFeedTime = 0;           //total feed seconds counter
    byte _intFeeder = LOW;
    byte _intFan = LOW;
    PitsBurnerMode _currentMode = MODE_IDLE;
    byte _intMinTemp = 0;                            //on HEAT cycle start set to current temp
    bool _boolIgnition = false;
    unsigned int _uiTimeWithoutFlame = 0; //milliseconds when flame was off (first time)
    PitsAlarmStatus _alarmStatus = ALARM_OK;
    bool _boolPump = false;
    
    
};

#endif




