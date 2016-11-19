/*
  pits_burner.h - Library for pellets burner control.
  Created by Rostislav Palivoda, October 15, 2016.
  GNU GPL3 Licence.
*/
#ifndef _PITSBURNER_H_
#define _PITSBURNER_H_

#include <TaskScheduler.h>
extern Scheduler scheduler;

class PitsBurner;
extern PitsBurner burner;

enum PitsBurnerMode {
  MODE_MANUAL,
  MODE_IGNITION,
  //MODE_PREHEAT, //when main circulation turns on we expect temperature drop up to 10 degree
  MODE_HEAT,
  MODE_IDLE,
  MODE_CLEANING,
  MODE_ALARM
};

enum PitsBurnerStatus {
  STATE_OK,
  STATE_TEMPDROP,
  STATE_NOFLAME,
  STATE_EXHAUST_HOT,
  STATE_IGNITION_FAILED,
  STATE_OVERHEAT
};

class PitsBurner
{
  public:
    PitsBurner();
    void init(int, int, int, int, int, int, int, int);
    static void onOperate();
    static void onFeed();
    static void onFan();
    static void onIgnite();
    void onAlarmOn();
    void onAlarmOff();
    
    void setRequiredTemp(int);
    int getRequiredTemp();
    void setCurrentTemp(int);  //to overwrite sensors value
    int getCurrentTemp();
    void setExhaustTemp(int);  //to overwrite sensors value
    int getExhaustTemp();
    void setFeederTemp(int);  //to overwrite sensors value
    int getFeederTemp();
    void setFlame(int);        //to overwrite sensors value
    int getFlame();
    bool isFlame();
    void setFeed(int);
    int getFeed();
    bool isFeed();
    void setFan(int);
    int getFan();
    bool isFan();
    int getFeedTime();
    void setIgnition(bool);
    bool isIgnition();
    void setStatus(PitsBurnerStatus);
    PitsBurnerStatus getStatus();

    int getSecondsWithoutFlame();
    void updateLastFlameStatus();
    void resetFlameTimer();

    PitsBurnerMode getCurrentMode();
    bool setCurrentMode(PitsBurnerMode);
    
  private:
    void _readSensors();
    void _switchMode();
    float _KTY81_210(float);
    float _LDR04(float);

    //physical pins connection set on object creation 
    int _pinTBoiler = 0;          //Analog in
    int _pinTExhaust = 0;         //Analog in
    int _pinFlameSensor = 0;      //Analog in
    int _pinTFeeder = 0;          //Analog in
    int _pinFan = 0;              //PWM out
    int _pinFeeder = 0;           //PWM out
    int _pinIgniter = 0;          //Digital out
    int _pinAlarm = 0;            //Digital out

    //sensors read values
    int _intCurrentTemp = 0;
    int _intExhaustTemp = 0;
    int _intFeederTemp = 0;
    int _intFlame = 0;
    
    //object internal states
    int _intFeedTime = 0;           //total feed seconds counter
    int _intFeeder = LOW;
    int _intFan = LOW;
    PitsBurnerMode _currentMode = MODE_IDLE;
    int _intMinTemp = 0;                            //on HEAT cycle start set to current temp
    bool _boolIgnition = false;
    unsigned long _longTimeWithoutFlame = 0; //milliseconds when flame was off (first time)
    PitsBurnerStatus _enumStatus = STATE_OK;
    
    //service settings
    const int _intMaxTemp = 103;
    const int _intMaxDropTemp = 10; 

    //user settings
    int _intRequiredTemp = 84;
    int _intHysteresisTemp = 2;
    
    int _intExhDeltaTemp = 11;
    int _intExhHysterisisTemp = 5;

    int _intIgniterStartS = 7;
    int _intIgniterDelayS = 8;
    int _intIgniterWorkS = 5;
    int _intFlameLevel = 5; 
    
    int _intFeedIgnitionWorkS = 1;
    int _intFeedIgnitionDelayS= 40;
    int _intFeedIgnitionP = 60;
    int _intFeedHeatWorkS = 1;
    int _intFeedHeatDelayS = 7;
    int _intFeedHeatP = 60;
    int _intFeedIdleWorkS = 1;
    int _intFeedIdleDelayS = 90;
    int _intFeedIdleP = 60;

    int _intFanIgnitionWorkS = 5; //TODO, added to UI
    int _intFanIgnitionOnP = 50;
    int _intFanIgnitionOffP = 30; //TODO, added to UI
    int _intFanHeatP = 60;
    int _intFanIdleOnP = 35; //if fan never turned off in idle then use this to kick start the fan, e.g > 25%
    int _intFanIdleWorkS = 5;  
    int _intFanIdleOffP = 10;  //if > 0 then never stop the fan in idle, used if smoke is blowing back without fan
    int _intFanCleanP = 100;

};

#endif




