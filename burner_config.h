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
  P100 = 100,
  P80 = 80,
  P60 = 60,
  P40 = 40,
  P20 = 20,
  P0 = 0,
};

struct BurnerConfigData {

    byte _version = 3; //data structure version

    //alarm page
    byte _intMaxTemp = 86;
    byte _intMaxDropTemp = 10; 

    //temp page
    byte _intRequiredTemp = 77;
    byte _intHysteresisTemp = 2;
    
    //exh page
    byte _intExhDeltaTemp = 40;

    //feed page
    byte _intFeedIgnitionWorkS = 1;
    byte _intFeedIgnitionDelayS= 40;
    byte _intFeedIgnitionP = 60;
    byte _intFeedHeatWorkS = 1;
    byte _intFeedHeatDelayS = 6;
    byte _intFeedHeatP = 60;
    byte _intFeedIdleWorkS = 1;
    byte _intFeedIdleDelayS = 90;
    byte _intFeedIdleP = 60;

    //fan page
    byte _intFanIgnitionWorkS = 5;   //TODO - in UI, add to logic
    byte _intFanIgnitionOnP = 50;
    byte _intFanIgnitionOffP = 30;   //TODO - in UI, add to logic
    byte _intFanHeatP = 70;
    byte _intFanIdleOnP = 35;        //if fan never turned off in idle then use this to kick start the fan, e.g > 25%
    byte _intFanIdleWorkS = 5;  
    byte _intFanIdleOffP = 10;       //if > 0 then never stop the fan in idle, used if smoke is blowing back without fan
    byte _intFanCleanWorkS = 10;     //TODO - in UI, add to logic
    byte _intFanCleanP = 100;        //TODO - in UI, add to logic

    //ignition page
    byte _intFlameLevel = 5; 
    byte _intFlameTimoutS = 180;   
    byte _intIgniterStartS = 7;
    byte _intIgniterDelayS = 8;
    byte _intIgniterWorkS = 5;

    //fuel (cm)
    byte _intFuel100 = 20;
    byte _intFuel80 = 40;
    byte _intFuel60 = 60;
    byte _intFuel40 = 80;
    byte _intFuel20 = 100;
    byte _intFuel0 = 120;
    
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
    uint16_t _netPin = 7532; //authorization for commands execution
    uint16_t _netId = 32767; //id for posting data to server
};

class BurnerConfig {
  
  private:
    BurnerConfigData _data;

  public:

    bool load();
    bool store();
    void reset();

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
    void setFanCleanWorkS(byte); 
    byte getFanCleanWorkS();
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
