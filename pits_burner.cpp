#include "Arduino.h"
#include "pits_burner.h"
#include <math.h>

PitsBurner burner;

Task tOperate(5*TASK_SECOND, TASK_FOREVER, &PitsBurner::onOperate, &scheduler, true);
Task tFeed(5*TASK_SECOND, TASK_FOREVER, &PitsBurner::onFeed, &scheduler, true);
Task tFan(5*TASK_SECOND, TASK_FOREVER, &PitsBurner::onFan, &scheduler, true);
Task tIgniter(1*TASK_SECOND, TASK_FOREVER, &PitsBurner::onIgnite, &scheduler, false);

PitsBurner::PitsBurner() {
  return;
}

void PitsBurner::init(int pinTBoiler, int pinTExhaust, int pinFlameSensor, int pinTFeeder, int pinFan, int pinFeeder, int pinIgniter, int pinAlarm) {

  Serial.print("PitsBurner: ");
  
  _pinTBoiler = pinTBoiler;
  pinMode(_pinTBoiler, INPUT);
  Serial.print("pinTBoiler=");
  Serial.print(_pinTBoiler);

  _pinTExhaust = pinTExhaust;
  pinMode(_pinTExhaust, INPUT);
  Serial.print(", pinTExhaust=");
  Serial.print(_pinTExhaust);

  _pinFlameSensor = pinFlameSensor;
  pinMode(_pinFlameSensor, INPUT);
  Serial.print(", pinFlameSensor=");
  Serial.print(_pinFlameSensor);

  _pinTFeeder = pinTFeeder;
  pinMode(_pinTFeeder, INPUT);
  Serial.print(", pinTFeeder=");
  Serial.print(_pinTFeeder);

  _pinFan = pinFan;
  pinMode(_pinFan, OUTPUT);
  analogWrite(_pinFan, LOW);
  Serial.print(", _pinFan=");
  Serial.print(_pinFan);
  
  _pinFeeder = pinFeeder;
  pinMode(_pinFeeder, OUTPUT);
  analogWrite(_pinFeeder, LOW);
  Serial.print(", pinFeeder=");
  Serial.print(_pinFeeder);

  _pinIgniter = pinIgniter;
  pinMode(_pinIgniter, OUTPUT);
  digitalWrite(_pinIgniter, LOW);
  Serial.print(", pinIgniter=");
  Serial.print(_pinIgniter);

  _pinAlarm = pinAlarm;
  pinMode(_pinAlarm, OUTPUT);
  digitalWrite(_pinAlarm, LOW);
  Serial.print(", pinAlarm=");
  Serial.print(pinAlarm);

  Serial.println(". Initiated.");
}



void PitsBurner::onOperate() {

   
  burner._readSensors();

  burner._switchMode();

}

void PitsBurner::_readSensors() {

  //pinMode(_pinFlameSensor, INPUT);
  setFlame(_LDR04(analogRead(_pinFlameSensor)));
  updateLastFlameStatus();
  
  //pinMode(_pinTBoiler, INPUT);
  setCurrentTemp(_KTY81_210(analogRead(_pinTBoiler)));
  //pinMode(_pinTExhaust, INPUT);
  setExhaustTemp(_KTY81_210(analogRead(_pinTExhaust)));
  //pinMode(_intFeederTemp, INPUT);
  setFeederTemp(_KTY81_210(analogRead(_pinTFeeder)));

  Serial.println(String("PitsBurner - ReadSensors: ") + 
    "CurrT=" + getCurrentTemp() + "C (" + (float(analogRead(_pinTBoiler)) / 1024 * 5) + "V) " + 
    "->" + getRequiredTemp() + "C, " + 
    "ExhaT=" + getExhaustTemp() + " (" + (float(analogRead(_pinTExhaust)) / 1024 * 5) + "V), " +  
    "Flame=" + getFlame() + "% (" + (float(analogRead(_pinFlameSensor)) / 1024 * 5) + "V), " + 
    "FeedT=" + getFeederTemp() + "C (" + (float(analogRead(_pinTFeeder)) / 1024 * 5) + "V), " +
    "NoFlame=" + getSecondsWithoutFlame() + "s"
    );
}

void PitsBurner::_switchMode() {

  //do nothing if manual mode
  if (_currentMode == MODE_MANUAL) return;

  //if temperature lower then minimum and not in ignition mode
  if (_intCurrentTemp < _intMinTemp && _currentMode == MODE_HEAT) {
    Serial.println(String("PitsBurner - SwitchMode - ALARM - current temperature ") + _intCurrentTemp + " drop under allowed minimum " + _intMinTemp + " during heat cycle"); 
    setStatus(STATE_TEMPDROP);
    setCurrentMode(MODE_ALARM);
  }

  //if no flame 3 minutes then switch to ignite
  //TODO: do not feed on switch to ignition - there already a lot of pellets to ignite.
  if (_currentMode == MODE_HEAT && getSecondsWithoutFlame() > 180) {
    Serial.println(String("PitsBurner - SwitchMode - IGNITE - no flame"));
    setStatus(STATE_NOFLAME);
    resetFlameTimer();
    setCurrentMode(MODE_IGNITION);
  }

  //if ignition and no flame for 10 minutes then alarm
  if (_currentMode == MODE_IGNITION && getSecondsWithoutFlame() > 10*60) {
    Serial.println(String("PitsBurner - SwitchMode - ALARM - flame timeout"));
    setStatus(STATE_IGNITION_FAILED);
    setCurrentMode(MODE_ALARM);
  }

  //if ignition and is flame then switch to heat 
  if (_currentMode == MODE_IGNITION && isFlame()) {
    Serial.println(String("PitsBurner - SwitchMode - HEAT - see flame.")); 
    setStatus(STATE_OK);
    setCurrentMode(MODE_HEAT);
  }

  //if under required temperature then heat
  if (_intCurrentTemp < (_intRequiredTemp  - _intHysteresisTemp)  && _currentMode == MODE_IDLE) {
    Serial.println(String("PitsBurner - SwitchMode - HEAT - under required temperature.")); 
    setStatus(STATE_OK);
    resetFlameTimer();
    setCurrentMode(MODE_HEAT);
  }


  //if exhaust sensor exists, if exhaust temperature more then  boiler temperature + allowed delta then idle
  //if (_intExhaustTemp > 0 && (_intExhaustTemp + _intExhDeltaTemp)  > _intCurrentTemp && _currentMode == MODE_HEAT) {
  //  Serial.println(String("PitsBurner - SwitchMode - IDLE because exhaust temperature")); 
  //  setStatus(STATE_OK);
  //  setCurrentMode(MODE_IDLE);
  //}

  //if under equired temperature during 1 hour then alarm


  //if heating and reached requied temperature then idle
  if (_intCurrentTemp > (_intRequiredTemp + _intHysteresisTemp)  && _currentMode == MODE_HEAT) {
    Serial.println(String("PitsBurner - SwitchMode - IDLE - required temperature reached.")); 
    setStatus(STATE_OK);
    setCurrentMode(MODE_IDLE);
  }

  //if in idle mode longer than 1 hour then alarm
  
  //if overheating then alarm
  if (_intCurrentTemp > _intMaxTemp && _currentMode != MODE_ALARM) {
    Serial.println(String("PitsBurner - SwitchMode - ALARM - overheat")); 
    setStatus(STATE_OVERHEAT);
    setCurrentMode(MODE_ALARM);
  }

}

void PitsBurner::setStatus(PitsBurnerStatus newStatus) {
   Serial.println(String("PitsBurner - Status: ") + newStatus);
  _enumStatus = newStatus;
}

PitsBurnerStatus PitsBurner::getStatus() {
  return _enumStatus;
}

void PitsBurner::setRequiredTemp(int t) {
  if (_intRequiredTemp  != t) {
    //Serial.println(String("PitsBurner - RequiredTemp: ") + _intRequiredTemp + "->" + t);
    _intRequiredTemp = t;
  }
}

int PitsBurner::getRequiredTemp() {
  return _intRequiredTemp;
}

void PitsBurner::setCurrentTemp(int t) {
  if (_intCurrentTemp != t) {
    //Serial.println(String("PitsBurner - CurrentTemp: ") + _intCurrentTemp + " -> " + t);
    _intCurrentTemp = t;
  }
}

int PitsBurner::getCurrentTemp() {
  return _intCurrentTemp;
}

void PitsBurner::setExhaustTemp(int t) {
  if (_intExhaustTemp != t) {
    //Serial.println(String("PitsBurner - ExhaustTemp: ") + _intExhaustTemp + " -> " + t);
    _intExhaustTemp = t;
  }
}

int PitsBurner::getExhaustTemp() {
  return _intExhaustTemp;
}

void PitsBurner::setFlame(int t) {
  //Serial.println(String("PitsBurner - Flame: ") + _intFlame + " to " + t);
  _intFlame = t;
}

void PitsBurner::updateLastFlameStatus() {
  if (isFlame()) {
    _longTimeWithoutFlame = 0;
  } else if (_longTimeWithoutFlame == 0) {
    _longTimeWithoutFlame = millis();
  }
}

int PitsBurner::getSecondsWithoutFlame() {
  if (_longTimeWithoutFlame == 0) return 0;

  unsigned long timeWithoutFlameMillis = millis() - _longTimeWithoutFlame;
  return (int) (timeWithoutFlameMillis / 1000);
}

void PitsBurner::resetFlameTimer() {
  Serial.println("PitsBurner - Reset Flame Timer");
  _longTimeWithoutFlame = 0;
}


int PitsBurner::getFlame() {
  return _intFlame;
}

bool PitsBurner::isFlame() {
  return _intFlame > _intFlameLevel; 
}

void PitsBurner::setFeederTemp(int t) {
  if (_intFeederTemp != t) {
    //Serial.println(String("PitsBurner - FeederTemp: ") + _intFeederTemp + " -> " + t);
    _intFeederTemp = t;
  }
}

int PitsBurner::getFeederTemp() {
  return _intFeederTemp;
}

PitsBurnerMode PitsBurner::getCurrentMode() {
  return _currentMode;
}

bool PitsBurner::setCurrentMode(PitsBurnerMode newMode) {
  Serial.println(String("PitsBurner - Mode: ") + _currentMode + " to " + newMode);

  //trigger alarm events
  if (MODE_ALARM == newMode) onAlarmOn();
  if (MODE_ALARM != newMode && MODE_ALARM == _currentMode) onAlarmOff();
  
  _currentMode = newMode;

  //feeder
  setFeed(LOW); 
  tFeed.restart();

  //fan
  setFan(LOW);
  tFan.restart();

  //ignition
  setIgnition(false);
  if (MODE_IGNITION == newMode) tIgniter.restart();
  else tIgniter.disable();
  
  //min temperature should not go down in heat mode
  _intMinTemp = (MODE_HEAT == newMode) ? _intCurrentTemp - _intMaxDropTemp : 0; 

}

//http://bildr.org/2012/03/rfp30n06le-arduino/
void PitsBurner::setFan(int percent) {
  burner._intFan = (int)((float(percent) / 100.0) * 255.0);
  analogWrite(_pinFan, burner._intFan); 
  Serial.println("PitsBurner: Fan is at " + String(percent) + "% = " + String(_intFan) + " PWM");
}

int PitsBurner::getFan() {
  return int(float(_intFan) / 255 * 100);
}

bool PitsBurner::isFan() {
  return _intFan != LOW;
}


void PitsBurner::onFan() {
  Serial.print("PitsBurner - OnFan: ");

  long interval;
  int percent;
  bool change = true;
  
  switch (burner._currentMode) {
    
    case MODE_MANUAL:
      Serial.print(" ignoring event in MODE_MANUAL, timer sleep 15s.");
      tFan.setInterval(15*TASK_SECOND);
      change = false;
      break;
      
    case MODE_IGNITION:
      Serial.print("in IGNITION ");
      interval = burner._intFeedIgnitionDelayS * TASK_SECOND + burner._intFeedIgnitionWorkS * TASK_SECOND; //sync to feed time
      percent = burner._intFanIgnitionOnP;
      Serial.print("turn ON ");
      break;
      
    case MODE_HEAT:
      Serial.print("in HEAT ");
      interval = (burner._intFeedHeatWorkS + burner._intFeedHeatDelayS) * TASK_SECOND; //sync to feed time
      percent = burner._intFanHeatP; //work all time
      Serial.print("turn ON ");
      break;
      
    case MODE_IDLE:
      Serial.print("in IDLE ");
      if (burner.isFan()) { 
        interval = burner._intFeedIdleDelayS * TASK_SECOND; //sync to feed time
        percent = burner._intFanIdleOffP; //LOW;
        Serial.print("turn OFF ");
      }
      else {
        interval = burner._intFeedIdleWorkS* TASK_SECOND; //sync to feed time
        percent = burner._intFanIdleOnP;
        Serial.print("turn ON ");
      }
      break;

    case MODE_CLEANING:
        interval = 3 * TASK_SECOND; //recheck burner status after 
        percent = burner._intFanCleanP;
      
      
    default: //in OTHER mode turn feed off
      Serial.print("in OTHER  ");
      interval = 60 * TASK_SECOND; 
      percent = LOW;
      break;
  }

  tFan.setInterval(interval);
  if (change) {
    Serial.print("during " + String(interval / TASK_SECOND) + " seconds ");
    burner.setFan(percent);
  }
  else {
    Serial.println();
  }
}

void PitsBurner::setFeed(int percent) {
  _intFeeder = int((float(percent) / 100) * 255);
  analogWrite(_pinFeeder, _intFeeder); 
  Serial.println("PitsBurner: Feed is at " + String(percent) + "% = " + String(_intFeeder) + " PWM");
}

int PitsBurner::getFeed() {
  return _intFeeder;
}

bool PitsBurner::isFeed() {
  return _intFeeder != LOW;
}

void PitsBurner::onFeed() {
  Serial.print("PitsBurner - OnFeed: ");

  long interval;
  int feed;
  bool change = true;
  
  switch (burner._currentMode) {
    case MODE_MANUAL:
      Serial.print(" ignoring event in MODE_MANUAL, timer sleep 15s.");
      tFeed.setInterval(15*TASK_SECOND);
      change = false;
      break;
      
    case MODE_IGNITION:
      Serial.print("in IGNITION ");
      if (burner.isFeed()) {
        interval = burner._intFeedIgnitionDelayS * TASK_SECOND;
        feed = LOW;
      }
      else {
        interval = burner._intFeedIgnitionWorkS * TASK_SECOND;
        feed = burner._intFeedIgnitionP;
      }
      break;
      
    case MODE_HEAT:
      Serial.print("in HEAT ");
      if (burner.isFeed()) {
        interval = burner._intFeedHeatDelayS * TASK_SECOND;
        feed = LOW;
        Serial.print("feed OFF ");
      }
      else {
        interval = burner._intFeedHeatWorkS * TASK_SECOND;
        feed = burner._intFeedHeatP;
        Serial.print("feed ON ");
      }
      break;
      
    case MODE_IDLE:
      Serial.print("in IDLE ");
      if (burner.isFeed()) {
        interval = burner._intFeedIdleDelayS * TASK_SECOND;
        feed = LOW;
      }
      else {
        interval = burner._intFeedIdleWorkS * TASK_SECOND;
        feed = burner._intFeedIdleP;
      }
      break;
      
    default: //in OTHER modes no feed at all
      Serial.print("in OTHER  ");
      interval = 60 * TASK_SECOND; 
      feed = LOW;
      break;
  }
  
  tFeed.setInterval(interval);
  
  if (change) {
    Serial.print("during " + String(interval / TASK_SECOND) + " seconds ");
    burner.setFeed(feed);

    //log total feed seconds
    if (feed != LOW) {
      burner._intFeedTime += (interval / TASK_SECOND);
    }
  }
  else {
    Serial.println();
  }
  
}

int PitsBurner::getFeedTime() {
  return _intFeedTime;
}

void PitsBurner::onIgnite() {

  if (MODE_IGNITION == burner._currentMode) {
    Serial.print("PitsBurner - OnIgnite: ");

    int interval = 0;
    bool ignite = false;

    interval = burner.isIgnition() ? burner._intIgniterDelayS : 
                        (tIgniter.isFirstIteration() ? burner._intIgniterStartS : burner._intIgniterWorkS);
    interval = interval * TASK_SECOND;
    ignite = !burner.isIgnition(); 

    tIgniter.setInterval(interval);
    Serial.print("during " + String(interval / TASK_SECOND) + " seconds ");
    
    burner.setIgnition(ignite);  
  }

}

void PitsBurner::setIgnition(bool turnOn) {
  _boolIgnition = turnOn;
  digitalWrite(_pinIgniter, turnOn ? HIGH : LOW); 
  Serial.println(String("PitsBurner: Ignition is ") + (turnOn ? "ON" : "OFF"));
}

bool PitsBurner::isIgnition() {
  return _boolIgnition;
}

void PitsBurner::onAlarmOn() {
  Serial.println("PitsBurner: OnAlarmOn");
  digitalWrite(_pinAlarm, HIGH);
}

void PitsBurner::onAlarmOff() {
  Serial.println("PitsBurner: OnAlarmOff");
  digitalWrite(_pinAlarm, LOW);
}

/*
Converts termistor KTY81-210 analog readings to celsius.
DataSheet - http://www.tme.eu/ru/Document/63412cca845bf05e8bcce2eecca1aa6d/KTY81-210.pdf
Codesample - http://electronics.stackexchange.com/questions/188813/strange-result-adcarduino-micro-thermistor-kty-10-6
Source - https://www.lemona.lv/?page=item&i_id=30742
*/                                
float PitsBurner::_KTY81_210(float sensorValue) {
  const int resistor = 2200; //2k2

  float ukty = 5 * sensorValue / 1023.0 ;
  float a = 0.00001874*1000;
  float b = 0.007884*1000;
  float c = 1000 - resistor * ukty / (5 - ukty);
  float delta = b * b - 4 * a * c;
  float delta1 = sqrt (delta);
  float x2 =(-b + delta1)/(2 * a);
  float temp1 = x2 + 25;
  return temp1;
 }

/*
Converts photo resistor LDR04 analog readings to percent.
DataSheet - http://www.velleman.eu/products/view/?country=be&lang=en&id=167303
*/
float PitsBurner::_LDR04(float sensorValue) {
  const int Res1 = 2200; // Resistor 1 value - 2k2 

  float Vout1 = sensorValue * (5.0/1024.0);      // calculate Voltage 1 (one unit = 5v / 1024)
  float Res2 = ((5.0  * Res1) / Vout1) - Res1;   // caclulate  Resistor 2
  return (Res2 / (Res1 + Res2)) * 100;
}


