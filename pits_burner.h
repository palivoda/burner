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
  MODE_STOP,
  MODE_IGNITION,
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
    void setFeed(bool);
    bool getFeed();
    void setFan(int);
    int getFan();

    PitsBurnerMode getCurrentMode();
    bool setCurrentMode(PitsBurnerMode);
    
  private:
    void _readSensors();
    void _switchMode();
    float _KTY81_110(float);
    float _LDR04(float);
    float _getInternalTemp();

    //pins
    int _pinTBoiler = 0;
    int _pinTExhaust = 0;
    int _pinFlameSensor = 0;
    int _pinTFeeder = 0;
    int _pinFan = 0;
    int _pinFeeder = 0;

    //read values: sensors, object state
    int _intCurrentTemp = 70;
    int _intExhaustTemp = 110;
    int _intFeederTemp = 23;
    int _intFlame = 0;
    bool _isFeed = false;
    int _intFan = 0;
    
    //service settings
    int _intMaxTemp = 90;
    int _intMinTemp = 40;

    //user settings
    int _intRequiredTemp = 70;
    int _intHysteresisTemp = 2;
    int _intExhDeltaTemp = 40;
    int _intFeedTimeIgnitionS = 30;
    int _intFeedDelayIgnitionS = 240;
    int _intFeedTimeHeatS = 30;
    int _intFeedDelayHeatS = 35;
    int _intFeedTimeIdelS = 5;
    int _intFeedDelayIdleS = 120;
    int _intFanIgnitionP = 20;
    int _intFanHeatP = 100;
    int _intFanIdleP = 70;
    int _intFanIdleWorkS = 20;

    PitsBurnerMode _currentMode = MODE_STOP;
};

#endif




