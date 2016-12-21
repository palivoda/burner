#include <EEPROM.h>

#include "burner_config.h"
BurnerConfig cfg;

#define EEPROM_ADDRESS 0

bool BurnerConfig::load() {
  #ifdef _BURNER_CONFIG_DEBUG
    Serial.println(F("Loading config"));
  #endif

  //if version changes then reset config
  byte eeVer = EEPROM.read(EEPROM_ADDRESS);
  #ifdef _BURNER_CONFIG_DEBUG
    Serial.print(F("Config versions:"));
    Serial.print(eeVer);
    Serial.print(F(" ?= "));
    Serial.println(_data._version);
  #endif
  if (eeVer != _data._version) reset();
  
  EEPROM.get(EEPROM_ADDRESS, _data);
  return (_data._version == 1);
}

bool BurnerConfig::store() {
  EEPROM.put(EEPROM_ADDRESS, _data);
  #ifdef _BURNER_CONFIG_DEBUG
    Serial.println(F("Config stored"));
  #endif
  return true;
}

void BurnerConfig::reset() {
  #ifdef _BURNER_CONFIG_DEBUG
    Serial.println(F("Config RESET"));
  #endif
  BurnerConfigData newData;
  _data = newData;
  store();
}

void BurnerConfig::setMaxTemp(byte v) 
{
  #ifdef _BURNER_CONFIG_DEBUG
    Serial.print(F("Max temp: "));
    Serial.println(v);
  #endif
  _data._intMaxTemp = v;
}

byte BurnerConfig::getMaxTemp()
{
  return _data._intMaxTemp;
}

void BurnerConfig::setMaxDropTemp(byte v)
{
  #ifdef _BURNER_CONFIG_DEBUG
    Serial.print(F("Max drop temp: "));
    Serial.println(v);
  #endif
  _data._intMaxDropTemp = v; 
}

byte BurnerConfig::getMaxDropTemp()
{
  return _data._intMaxDropTemp;
}

void BurnerConfig::setRequiredTemp(byte t) {
  if (_data._intRequiredTemp  != t) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("RequiredTemp: "));
      Serial.print(_data._intRequiredTemp);
      Serial.print(F("->"));
      Serial.println(t);
    #endif
    _data._intRequiredTemp = t;
  }
}

byte BurnerConfig::getRequiredTemp() {
  return _data._intRequiredTemp;
}

void BurnerConfig::setHysteresisTemp(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("Histeresys: "));
      Serial.println(_data._intHysteresisTemp);
    #endif
    _data._intHysteresisTemp = v;
}

byte BurnerConfig::getHysteresisTemp() {
  return _data._intHysteresisTemp;
}

void BurnerConfig::setExhaustDeltaTemp(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("ExhaustDeltaTemp: "));
      Serial.println(_data._intExhDeltaTemp);
    #endif
    _data._intExhDeltaTemp = v;
}

byte BurnerConfig::getExhaustDeltaTemp() {
  return _data._intExhDeltaTemp;
}

void BurnerConfig::setFeedIgnitionWorkS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedIgnitionWorkS: "));
      Serial.println(v);
    #endif
    _data._intFeedIgnitionWorkS = v;
}

byte BurnerConfig::getFeedIgnitionWorkS() {
  return _data._intFeedIgnitionWorkS;
}

void BurnerConfig::setFeedIgnitionDelayS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedIgnitionDelayS: "));
      Serial.println(v);
    #endif
    _data._intFeedIgnitionDelayS = v;
}

byte BurnerConfig::getFeedIgnitionDelayS() {
  return _data._intFeedIgnitionDelayS;
}

void BurnerConfig::setFeedIgnitionP(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedIgnitionP: "));
      Serial.println(v);
    #endif
    _data._intFeedIgnitionP = v;
}

byte BurnerConfig::getFeedIgnitionP() {
  return _data._intFeedIgnitionP;
}

void BurnerConfig::setFeedHeatWorkS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedHeatWorkS: "));
      Serial.println(v);
    #endif
    _data._intFeedHeatWorkS = v;
}

byte BurnerConfig::getFeedHeatWorkS() {
  return _data._intFeedHeatWorkS;
}

void BurnerConfig::setFeedHeatDelayS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedHeatDelayS: "));
      Serial.println(v);
    #endif
    _data._intFeedHeatDelayS = v;
}

byte BurnerConfig::getFeedHeatDelayS() {
  return _data._intFeedHeatDelayS;
}

void BurnerConfig::setFeedHeatP(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedHeatP: "));
      Serial.println(v);
    #endif
    _data._intFeedHeatP = v;
}

byte BurnerConfig::getFeedHeatP() {
  return _data._intFeedHeatP;
}

void BurnerConfig::setFeedIdleWorkS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedIdleWorkS: "));
      Serial.println(v);
    #endif
    _data._intFeedIdleWorkS = v;
}

byte BurnerConfig::getFeedIdleWorkS() {
  return _data._intFeedIdleWorkS;
}

void BurnerConfig::setFeedIdleDelayS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedIdleDelayS: "));
      Serial.println(v);
    #endif
    _data._intFeedIdleDelayS = v;
}

byte BurnerConfig::getFeedIdleDelayS() {
  return _data._intFeedIdleDelayS;
}

void BurnerConfig::setFeedIdleP(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedIdleP: "));
      Serial.println(v);
    #endif
    _data._intFeedIdleP = v;
}

byte BurnerConfig::getFeedIdleP() {
  return _data._intFeedIdleP;
}

void BurnerConfig::setFeedAmpsMax(float v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedAmpsMax: "));
      Serial.println(v);
    #endif
    _data._flFeedAmpsMax= v;
}

float BurnerConfig::getFeedAmpsMax() {
  return _data._flFeedAmpsMax;
}

void BurnerConfig::setFeedAmpsRev(float v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FeedAmpsRev: "));
      Serial.println(v);
    #endif
    _data._flFeedAmpsRev= v;
}

float BurnerConfig::getFeedAmpsRev() {
  return _data._flFeedAmpsRev;
}

void BurnerConfig::setFanIgnitionWorkS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FanIgnitionWorkS: "));
      Serial.println(v);
    #endif
    _data._intFanIgnitionWorkS = v;
}

byte BurnerConfig::getFanIgnitionWorkS() {
  return _data._intFanIgnitionWorkS;
}

void BurnerConfig::setFanIgnitionOnP(byte v) { 
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FanIgnitionOnP: "));
      Serial.println(v);
    #endif
    _data._intFanIgnitionOnP = v;
}

byte BurnerConfig::getFanIgnitionOnP() {
  return _data._intFanIgnitionOnP;
}

void BurnerConfig::setFanIgnitionOffP(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FanIgnitionOffP: "));
      Serial.println(v);
    #endif
    _data._intFanIgnitionOffP = v;
}

byte BurnerConfig::getFanIgnitionOffP() {
  return _data._intFanIgnitionOffP;
}

void BurnerConfig::setFanHeatP(byte v) { 
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FanHeatP: "));
      Serial.println(v);
    #endif
    _data._intFanHeatP = v;
}

byte BurnerConfig::getFanHeatP() { 
  return _data._intFanHeatP;
}

void BurnerConfig::setFanIdleOnP(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FanIdleOnP: "));
      Serial.println(v);
    #endif
    _data._intFanIdleOnP = v;
}

byte BurnerConfig::getFanIdleOnP() { 
  return _data._intFanIdleOnP;
}

void BurnerConfig::setFanIdleWorkS(byte v) { 
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FanIdleWorkS: "));
      Serial.println(v);
    #endif
    _data._intFanIdleWorkS = v;
}

byte BurnerConfig::getFanIdleWorkS() {
  return _data._intFanIdleWorkS;
}

void BurnerConfig::setFanIdleOffP(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FanIdleOffP: "));
      Serial.println(v);
    #endif
    _data._intFanIdleOffP = v;
}

byte BurnerConfig::getFanIdleOffP() {
  return _data._intFanIdleOffP;
}

void BurnerConfig::setFanCleanP(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FanCleanP: "));
      Serial.println(v);
    #endif
    _data._intFanCleanP = v;
} 

byte BurnerConfig::getFanCleanP() {
  return _data._intFanCleanP;
}

void BurnerConfig::setFanCleanWorkS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FanCleanWorkS: "));
      Serial.println(v);
    #endif
    _data._intFanCleanWorkS = v;
} 

byte BurnerConfig::getFanCleanWorkS() {
  return _data._intFanCleanWorkS;
}

void BurnerConfig::setIgniterStartS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("IgniterStartS: "));
      Serial.println(v);
    #endif
    _data._intIgniterStartS = v;
}

byte BurnerConfig::getIgniterStartS() {
  return _data._intIgniterStartS;
}

void BurnerConfig::setIgniterDelayS(byte v) { 
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("IgniterDelayS: "));
      Serial.println(v);
    #endif
    _data._intIgniterDelayS = v;
}

byte BurnerConfig::getIgniterDelayS() {
  return _data._intIgniterDelayS;
}

void BurnerConfig::setIgniterWorkS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("IgniterWorkS: "));
      Serial.println(v);
    #endif
    _data._intIgniterWorkS = v;
}

byte BurnerConfig::getIgniterWorkS() { 
  return _data._intIgniterWorkS;
}

void BurnerConfig::setFlameLevel(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FlameLevel: "));
      Serial.println(v);
    #endif
    _data._intFlameLevel = v;
}

byte BurnerConfig::getFlameLevel() {
  return _data._intFlameLevel;
}

void BurnerConfig::setFlameTimoutS(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("FlameTimoutS: "));
      Serial.println(v);
    #endif
    _data._intFlameTimoutS = v;
}

byte BurnerConfig::getFlameTimoutS() {
  return _data._intFlameTimoutS;
}

void BurnerConfig::setFuelLevel(PERCENT_RANGE l, byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("Fuel "));
      Serial.print(l);
      Serial.print(F(": "));
      Serial.println(v);
    #endif
    switch (l) {
      case P100: _data._intFuel100 = v; break;
      case P80:  _data._intFuel80 = v;  break;
      case P60:  _data._intFuel60 = v;  break;
      case P40:  _data._intFuel40 = v;  break;
      case P20:  _data._intFuel20 = v;  break;
      case P0:   _data._intFuel0 = v;   break;
    }
}

byte BurnerConfig::getFuelLevel(PERCENT_RANGE l) {
    switch (l) {
      case P100: return _data._intFuel100;
      case P80:  return _data._intFuel80; 
      case P60:  return _data._intFuel60; 
      case P40:  return _data._intFuel40; 
      case P20:  return _data._intFuel20; 
      case P0:   return _data._intFuel0;  
    }
}

void BurnerConfig::setBattLevel(PERCENT_RANGE l, byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("Batt "));
      Serial.print(l);
      Serial.print(F(": "));
      Serial.println(v);
    #endif
    switch (l) {
      case P100: _data._intBatt100 = v; break;
      case P80:  _data._intBatt80 = v;  break;
      case P60:  _data._intBatt60 = v;  break;
      case P40:  _data._intBatt40 = v;  break;
      case P20:  _data._intBatt20 = v;  break;
      case P0:   _data._intBatt0 = v;   break;
    }
}

byte BurnerConfig::getBattLevel(PERCENT_RANGE l) {
    switch (l) {
      case P100: return _data._intBatt100;
      case P80:  return _data._intBatt80; 
      case P60:  return _data._intBatt60; 
      case P40:  return _data._intBatt40; 
      case P20:  return _data._intBatt20; 
      case P0:   return _data._intBatt0;  
    }
}

void BurnerConfig::setPumpOnTemp(byte v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("Pump temp: "));
      Serial.println(v);
    #endif
    _data._intPumpOnTemp = v;
}

byte BurnerConfig::getPumpOnTemp() {
  return _data._intPumpOnTemp;
}

void BurnerConfig::setNetPin(uint16_t v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("Net PIN: "));
      Serial.println(v);
    #endif
    _data._netPin = v;
}

uint16_t BurnerConfig::getNetPin() { 
  return _data._netPin;
}

void BurnerConfig::setNetId(uint16_t v) {
    #ifdef _BURNER_CONFIG_DEBUG
      Serial.print(F("Net ID: "));
      Serial.println(v);
    #endif
    _data._netId = v;
}

uint16_t BurnerConfig::getNetId() { 
  return _data._netId;
}


