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
  MODE_PREHEAT, //when main circulation turns on we expect temperature drop up to 10 degree
  MODE_HEAT,
  MODE_IDLE,
  MODE_CLEANING,
  MODE_ALARM
};


class PitsBurner
{
  public:
    PitsBurner();
    void init(int, int, int, int, int, int);
    static void onOperate();
    static void onFeed();
    static void onFan();
    
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

    PitsBurnerMode getCurrentMode();
    bool setCurrentMode(PitsBurnerMode);
    
  private:
    void _readSensors();
    void _switchMode();
    float _KTY81_210(float);
    float _LDR04(float);

    //pins
    int _pinTBoiler = 0;
    int _pinTExhaust = 0;
    int _pinFlameSensor = 0;
    int _pinTFeeder = 0;
    int _pinFan = 0;
    int _pinFeeder = 0;

    //read values
    int _intCurrentTemp = 0;
    int _intExhaustTemp = 0;
    int _intFeederTemp = 0;
    int _intFlame = 0;
    
    //object state
    int _intFeeder = LOW;
    int _intFan = LOW;
    
    //service settings
    int _intMaxTemp = 95;
    int _intMinTemp = 70;

    //user settings
    int _intRequiredTemp = 85;
    int _intHysteresisTemp = 2;
    int _intExhDeltaTemp = 40;
    
    int _intFeedIgnitionP = 60;
    int _intFeedIgnitionWorkS = 30;
    int _intFeedIgnitionDelayS= 240;
    int _intFeedHeatWorkS = 1;
    int _intFeedHeatDelayS = 7;
    int _intFeedIdleWorkS = 1;
    int _intFeedIdleDelayS = 90;
    int _intFeedHeatP = 60;
    int _intFeedIdleP = 60;
    int _intFanIgnitionP = 40;
    int _intFanHeatP = 60;
    int _intFanIdleWorkP = 35; //should be strong enougth to start fan, e.g > 25%
    int _intFanIdleWorkS = 5;
    int _intFanIdleOffP = 10; //never stop the fan in idle if smoke is blowing back without fan
    int _intFanCleanP = 80;

    PitsBurnerMode _currentMode = MODE_IDLE;
};

#endif




