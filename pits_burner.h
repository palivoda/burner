/*
  pits_burner - Class for pellets burner control.
  Created by Rostislav Palivoda, October 15, 2016.
  GNU GPL3 Licence.
*/
#ifndef _PITSBURNER_H_
#define _PITSBURNER_H_

#include <Arduino.h>

#include "burner_config.h"

#define _BURNER_DEBUG_SERIAL_
//#define _BURNER_SET_DEBUG_SERIAL_

enum PitsBurnerMode {
  MODE_MANUAL = 1,
  MODE_IGNITION = 2,
  MODE_HEAT = 3,
  MODE_IDLE = 4,
  MODE_CLEANING = 5,
  MODE_ALARM = 6
};

enum PitsAlarmStatus { //numbers used in display pMain.tmUpdate
  ALARM_OK = 1,
  ALARM_MANUAL = 2, 
  ALARM_TEMPDROP = 3,
  ALARM_NOFLAME = 4,
  ALARM_IGNITION_FAILED = 5,
  ALARM_OVERHEAT = 6,
  ALARM_FEED_STUCK = 7, 
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
    void setFeederAmps(float);   //to overwrite sensors value
    float getFeederAmps();
    
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
    bool isFeedReverse();
    void setFeedReverse(bool); 
    
    void setIgnition(bool);
    bool isIgnition();

    PERCENT_RANGE getFuelLevel();
    byte getFuelCm();
    void setFuelLevel(PERCENT_RANGE);

    PERCENT_RANGE getBattLevel();
    byte getBattDVolts();
    void setBattLevel(PERCENT_RANGE);
    
    bool isPumpUPS();
    void setPumpUPS(bool);
    
    short getSecondsWithoutFlame();
    void updateLastFlameStatus();
    void resetFlameTimer();

    PitsAlarmStatus getAlarmStatus();
    PitsBurnerMode getCurrentMode();
    void setCurrentMode(PitsBurnerMode, PitsAlarmStatus);
    short getSecondsInCurrentMode();
    
  private:
    void _readSensors();
    void _switchMode();
    void _inAlarmOperate();
    float _KTY81_110(byte);
    float _LDR04(byte);
    void _setAlarmStatus(PitsAlarmStatus);
    byte _HYSRF05(byte, byte);
    float _vDivVin(float, float, byte);
    float _ACS712(byte);

    //physical pins connection
    const byte _pinTBoiler = A7;          //Analog in
    const byte _pinTFeeder = A6;          //Analog in
    const byte _pinTExhaust = A5;         //Analog in
    const byte _pinFlameSensor = A4;      //Analog in
    const byte _pinBattery = A3;          //Analog in   
    //const byte _pinLambda = A2;         //Analog in, TODO
    const byte _pinFeedAmps = A1;         //Analog in, TODO: test
    const byte _pinFeedReverse = A0;      //Digital out
    const byte _pinAlarm = 2;             //Digital out
    const byte _pinIgniter = 3;           //Digital out
    const byte _pinBuzzer = 4;            //Digital in
    const byte _pinFan =  5;              //Digital PWM out
    const byte _pinFeeder = 6;            //Digital PWM out
    const byte _pinFuelTrig = 7;          //Digital out
    const byte _pinFuelEcho = 8;          //Digital in
    const byte _pinPumpUPS = 9;           //Digital out

    //sensors read values
    byte _intCurrentTemp = 0;
    byte _intExhaustTemp = 0;
    byte _intFeederTemp = 0;
    byte _intFlame = 0;
    PERCENT_RANGE _intBattLevel = P0;  
    byte _intBattDVolts = 0;  
    PERCENT_RANGE _intFuelLevel = P0;
    byte _intFuelCm = 0;  
    float _flFeedAmps[3] = {0,0,0};
    
    //object internal states
    unsigned int _intFeedTime = 0;            //total feed seconds counter, reset on power off
    byte _intFeeder = LOW;
    byte _intFan = LOW;
    PitsBurnerMode _currentMode = MODE_IDLE;
    byte _intMinTemp = 0;                     //on HEAT cycle start set to current temp
    bool _boolIgnition = false;
    short _uiTimeWithoutFlame = 0;            //seconds when flame was off (first time)
    PitsAlarmStatus _alarmStatus = ALARM_OK;
    bool _boolPumpUPS = false;
    bool _boolFeedRev = false;
    short _uiModeTimer = 0;                //duration in current mode, 0 if not
        
};

#endif
