/*
  burner_config - Stores burner configuration values in Arduino EEPROM.
  Created by Rostislav Palivoda, December 5, 2016.
  GNU GPL3 Licence.
 */
#ifndef _BURNER_CONFIG_H_
#define _BURNER_CONFIG_H_

#include <Arduino.h>

//#define _BURNER_CONFIG_DEBUG

enum PERCENT_RANGE {
  CHARGE = 101,
  P100 = 100,
  P80 = 80,
  P60 = 60,
  P40 = 40,
  P20 = 20,
  P0 = 0,
};

struct BurnerConfigData {

    byte _version = 7; //data structure version
    uint32_t _hid=0;

    //alarm page
    byte _intMaxTemp = 95;
    byte _intMaxDropTemp = 20; 

    //temp page
    byte _intRequiredTemp = 75;
    byte _intHysteresisTemp = 1;
    
    //exh page
    byte _intExhDeltaTemp = 35; //TODO: add Max Exhaust and Min Exhaust deltas. 

    //feed page
    byte _intFeedIgnitionWorkS = 1;
    byte _intFeedIgnitionDelayS= 60;
    byte _intFeedIgnitionP = 70;
    byte _intFeedHeatWorkS = 1;
    byte _intFeedHeatDelayS = 8;
    byte _intFeedHeatP = 70;
    byte _intFeedIdleWorkS = 1;
    byte _intFeedIdleDelayS = 160;
    byte _intFeedIdleP = 70;
    byte _flFeedAmpsRev = 55; 
    byte _flFeedAmpsMax = 80; 

    //fan page
    byte _intFanIgnitionWorkS = 5;   
    byte _intFanIgnitionOnP = 50;
    byte _intFanIgnitionOffP = 30;   
    byte _intFanHeatP = 70;
    byte _intFanIdleWorkS = 5;  
    byte _intFanIdleOnP = 35;        //if fan never turned off in idle then use this to kick start the fan, e.g > 25%
    byte _intFanIdleOffP = 12;       //if > 0 then never stop the fan in idle, used if smoke is blowing back without fan
    short _intFanCleanWorkS = 600;     
    byte _intFanCleanP = 100;        

    //ignition page
    byte _intFlameLevel = 15; 
    short _intFlameTimoutS = 240;   
    byte _intIgniterStartS = 8;
    byte _intIgniterDelayS = 5;
    byte _intIgniterWorkS = 6;

    //fuel (cm)
    byte _intFuel100 = 33;
    byte _intFuel80 = 47;
    byte _intFuel60 = 61;
    byte _intFuel40 = 75;
    byte _intFuel20 = 89;
    byte _intFuel0 = 103;
    
    //battery (dcV)
    byte _intBatt100 = 127;
    byte _intBatt80 = 125;
    byte _intBatt60 = 122;
    byte _intBatt40 = 119;
    byte _intBatt20 = 115;
    byte _intBatt0 = 105;

    //pump
    byte _intPumpOnTemp = 40; //TODO: not in UI

    //ethernet
    short _netPin = 7532; //authorization for commands execution
    short _netId = 32767; //id for posting data to server
};

class BurnerConfig {
  
  private:
    BurnerConfigData _data;

  public:

    bool load();
    bool store();
    void reset();

    uint32_t getHID();
    void generateHID();

    void setMaxTemp(byte);
    byte getMaxTemp();
    void setMaxDropTemp(byte);
    byte getMaxDropTemp();
    
    void setRequiredTemp(byte);
    byte getRequiredTemp();
    void setHysteresisTemp(byte);
    byte getHysteresisTemp();
    
    void setExhaustDeltaTemp(byte);
    byte getExhaustDeltaTemp();
    
    void setFeedIgnitionWorkS(byte);
    byte getFeedIgnitionWorkS();
    void setFeedIgnitionDelayS(byte);
    byte getFeedIgnitionDelayS();
    void setFeedIgnitionP(byte);
    byte getFeedIgnitionP();
    void setFeedHeatWorkS(byte);
    byte getFeedHeatWorkS();
    void setFeedHeatDelayS(byte);
    byte getFeedHeatDelayS();
    void setFeedHeatP(byte);
    byte getFeedHeatP();
    void setFeedIdleWorkS(byte);
    byte getFeedIdleWorkS();
    void setFeedIdleDelayS(byte);
    byte getFeedIdleDelayS();
    void setFeedIdleP(byte);
    byte getFeedIdleP();
    void setFeedAmpsRev(float);
    float getFeedAmpsRev();
    void setFeedAmpsMax(float);
    float getFeedAmpsMax();
    
    void setFanIgnitionWorkS(byte);
    byte getFanIgnitionWorkS();
    void setFanIgnitionOnP(byte);
    byte getFanIgnitionOnP();
    void setFanIgnitionOffP(byte);
    byte getFanIgnitionOffP();
    void setFanHeatP(byte);
    byte getFanHeatP();
    void setFanIdleWorkS(byte);
    byte getFanIdleWorkS();
    void setFanIdleOnP(byte);
    byte getFanIdleOnP();
    void setFanIdleOffP(byte);
    byte getFanIdleOffP();
    void setFanCleanWorkS(short); 
    short getFanCleanWorkS();
    void setFanCleanP(byte); 
    byte getFanCleanP();
    
    void setIgniterStartS(byte);
    byte getIgniterStartS();
    void setIgniterDelayS(byte);
    byte getIgniterDelayS();
    void setIgniterWorkS(byte);
    byte getIgniterWorkS();
    
    void setFlameLevel(byte);
    byte getFlameLevel();
    void setFlameTimoutS(byte);
    byte getFlameTimoutS();
    
    void setFuelLevel(PERCENT_RANGE, byte);
    byte getFuelLevel(PERCENT_RANGE);
    
    void setBattLevel(PERCENT_RANGE, byte);
    byte getBattLevel(PERCENT_RANGE);

    void setPumpOnTemp(byte);
    byte getPumpOnTemp();

    uint16_t getNetPin();
    void setNetPin(uint16_t);
    uint16_t getNetId();
    void setNetId(uint16_t);
};

#endif
