#include "pits_burner.h"
PitsBurner burner;

#include "burner_config.h"
extern BurnerConfig cfg;

#include <TaskScheduler.h>
extern Scheduler scheduler;

Task tFeed(5*TASK_SECOND, TASK_FOREVER, &PitsBurner::onFeed, &scheduler, true);
Task tFan(5*TASK_SECOND, TASK_FOREVER, &PitsBurner::onFan, &scheduler, true);
Task tIgniter(1*TASK_SECOND, TASK_FOREVER, &PitsBurner::onIgnite, &scheduler, false);

void PitsBurner::init() {
   pinMode(_pinTBoiler,INPUT);  
   pinMode(_pinTExhaust,INPUT);  
   pinMode(_pinFlameSensor,INPUT);  
   pinMode(_pinTFeeder,INPUT);  
   pinMode(_pinFan,OUTPUT);  
   pinMode(_pinFeeder,OUTPUT);  
   pinMode(_pinIgniter,OUTPUT);
   pinMode(_pinAlarm,OUTPUT);
   pinMode(_pinFuelTrig,OUTPUT);
   pinMode(_pinFuelEcho,INPUT);
   pinMode(_pinBattery,OUTPUT);
   pinMode(_pinPump,OUTPUT);
}

void PitsBurner::operate() {
  _readSensors();
  _switchMode();
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
  setBattLevel(random(80,100)); //TODO

  byte fuelCm = _HYSRF05();
  if (fuelCm > cfg.getFuelLevel(P0)) setFuelLevel(P0);
  else if (fuelCm > cfg.getFuelLevel(P20)) setFuelLevel(P20);
  else if (fuelCm > cfg.getFuelLevel(P40)) setFuelLevel(P40);
  else if (fuelCm > cfg.getFuelLevel(P60)) setFuelLevel(P60);
  else if (fuelCm > cfg.getFuelLevel(P80)) setFuelLevel(P80);
  else setFuelLevel(P100);

#ifdef _BURNER_DEBUG_SERIAL_
  Serial.println(String("ReadSensors: ") + 
    F("CurT=") + getCurrentTemp() + "C (" + (float(analogRead(_pinTBoiler)) / 1024 * 5) + "V) " + 
    F("->") + cfg.getRequiredTemp() + "C, " + 
    F("ExhT=") + getExhaustTemp() + " (" + (float(analogRead(_pinTExhaust)) / 1024 * 5) + "V), " +  
    F("Flame=") + getFlame() + "% (" + (float(analogRead(_pinFlameSensor)) / 1024 * 5) + "V), " + 
    F("FeedT=") + getFeederTemp() + "C (" + (float(analogRead(_pinTFeeder)) / 1024 * 5) + "V), " +
    F("NoFlame=") + getSecondsWithoutFlame() + "s, " + 
    F("Fuel=") + getFuelLevel() + "% (" + fuelCm + ")"
    );
#endif

}

void PitsBurner::_switchMode() {

  //do nothing if manual mode
  if (_currentMode == MODE_MANUAL) return;

  //if temperature lower then minimum and not in ignition mode
  //NOTE1: when main circulation turns on we expect temperature drop up to 10 degree
  //NOTE2: _intMinTemp set to "cur temp" - "alarm drop temp" in the begining of heat cycle
  if ( _intCurrentTemp < _intMinTemp && _currentMode == MODE_HEAT) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.print(F("SwitchMode-ALARM-cur temp "));
      Serial.print(_intCurrentTemp);
      Serial.print(F(" is under allowed minimum "));
      Serial.print(_intMinTemp);
      Serial.println(F(" during heat cycle"));
    #endif
    setCurrentMode(MODE_ALARM, ALARM_TEMPDROP);
  }

  //control circulation pump
  if ( _intCurrentTemp > cfg.getPumpOnTemp() + cfg.getHysteresisTemp() && _currentMode != MODE_MANUAL) {
    setPump(true);
  }
  else if (_intCurrentTemp < _intMinTemp - cfg.getHysteresisTemp()) {
    setPump(false);
  }

  //if no flame 3 minutes then switch to ignite
  //TODO: do not feed on switch to ignition - there already a lot of pellets to ignite.
  if (_currentMode == MODE_HEAT && getSecondsWithoutFlame() > cfg.getFlameTimoutS()) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.println(F("SwitchMode-IGNITE-no flame"));
    #endif
    resetFlameTimer();
    setCurrentMode(MODE_IGNITION, ALARM_NOFLAME);
  }

  //if ignition and no flame for 10 minutes then alarm
  if (_currentMode == MODE_IGNITION && getSecondsWithoutFlame() > 10*60) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.println(F("SwitchMode-ALARM-flame timeout"));
    #endif
    setCurrentMode(MODE_ALARM, ALARM_IGNITION_FAILED);
  }

  //if ignition and is flame then switch to heat 
  if (_currentMode == MODE_IGNITION && isFlame()) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.println(F("SwitchMode-HEAT-see flame")); 
    #endif
    setCurrentMode(MODE_HEAT, ALARM_OK);
  }

  //if under required temperature then heat
  if (_intCurrentTemp < (cfg.getRequiredTemp()  - cfg.getHysteresisTemp())  && _currentMode == MODE_IDLE) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.println(F("SwitchMode-HEAT-under required temperature")); 
    #endif
    resetFlameTimer();
    setCurrentMode(MODE_HEAT, ALARM_OK);
  }


  //if exhaust sensor exists, if exhaust temperature more then  boiler temperature + allowed delta then idle
  //if (_intExhaustTemp > 0 && (_intExhaustTemp + _intExhDeltaTemp)  > _intCurrentTemp && _currentMode == MODE_HEAT) {
  //  #ifdef _BURNER_DEBUG_SERIAL_
  //    Serial.println(F("SwitchMode-IDLE because exhaust temperature")); 
  //  #endif
  //  setCurrentMode(MODE_IDLE, ALARM_OK);
  //}

  //if under equired temperature during 1 hour then alarm


  //if heating and reached requied temperature then idle
  if (_intCurrentTemp > (cfg.getRequiredTemp() + cfg.getHysteresisTemp())  && _currentMode == MODE_HEAT) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.println(F("SwitchMode-IDLE-temp reached")); 
    #endif
    setCurrentMode(MODE_IDLE, ALARM_OK);
  }

  //TODO: if in idle mode longer than 1 hour then alarm
  
  //if overheating then alarm
  if (_intCurrentTemp > cfg.getMaxTemp() && _currentMode != MODE_ALARM) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.println(F("SwitchMode-ALARM - overheat")); 
    #endif
    setCurrentMode(MODE_ALARM, ALARM_OVERHEAT);
  }

}

void PitsBurner::_setAlarmStatus(PitsAlarmStatus newStatus) {
  #ifdef _BURNER_SET_DEBUG_SERIAL_
    Serial.print(F("Alarm status: "));
    Serial.println(newStatus);
  #endif
  _alarmStatus = newStatus;
}

PitsAlarmStatus PitsBurner::getAlarmStatus() {
  return _alarmStatus;
}

void PitsBurner::setCurrentTemp(byte t) {
  if (_intCurrentTemp != t) {
    #ifdef _BURNER_SET_DEBUG_SERIAL_
      Serial.print(F("CurrentTemp: "));
      Serial.print(_intCurrentTemp);
      Serial.print(F("->"));
      Serial.println(t);
    #endif
    _intCurrentTemp = t;
  }
}

byte PitsBurner::getCurrentTemp() {
  return _intCurrentTemp;
}

void PitsBurner::setExhaustTemp(byte t) {
  if (_intExhaustTemp != t) {
    #ifdef _BURNER_SET_DEBUG_SERIAL_
      Serial.print(F("ExhaustTemp: "));
      Serial.print(_intExhaustTemp);
      Serial.print(F("->"));
      Serial.println(t);
    #endif
    _intExhaustTemp = t;
  }
}

byte PitsBurner::getExhaustTemp() {
  return _intExhaustTemp;
}

void PitsBurner::setFlame(byte t) {
    #ifdef _BURNER_SET_DEBUG_SERIAL_
      Serial.print(F("Flame: "));
      Serial.print(_intFlame);
      Serial.print(F("->"));
      Serial.println(t);
    #endif
  _intFlame = t;
}

void PitsBurner::updateLastFlameStatus() {
  if (isFlame()) {
    _uiTimeWithoutFlame = 0;
  } else if (_uiTimeWithoutFlame == 0) {
    _uiTimeWithoutFlame = millis();
  }
}

unsigned int PitsBurner::getSecondsWithoutFlame() {
  if (_uiTimeWithoutFlame == 0) return 0;

  unsigned long timeWithoutFlameMillis = millis() - _uiTimeWithoutFlame;
  return (unsigned int) (timeWithoutFlameMillis / 1000);
}

void PitsBurner::resetFlameTimer() {
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.println("PitsBurner - Reset Flame Timer");
  #endif
  _uiTimeWithoutFlame = 0;
}


byte PitsBurner::getFlame() {
  return _intFlame;
}

bool PitsBurner::isFlame() {
  return _intFlame > cfg.getFlameLevel(); 
}

void PitsBurner::setFeederTemp(byte t) {
  if (_intFeederTemp != t) {
    #ifdef _BURNER_SET_DEBUG_SERIAL_
      Serial.print(F("FeederTemp: "));
      Serial.print(_intFeederTemp);
      Serial.print(F("->"));
      Serial.println(t);
    #endif
    _intFeederTemp = t;
  }
}

byte PitsBurner::getFeederTemp() {
  return _intFeederTemp;
}

PitsBurnerMode PitsBurner::getCurrentMode() {
  return _currentMode;
}

bool PitsBurner::setCurrentMode(PitsBurnerMode newMode, PitsAlarmStatus newStatus) {
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.print(F("Mode change: "));
    Serial.println(newMode);
  #endif
  
  //trigger alarm events
  if (MODE_ALARM == newMode) {
    _setAlarmStatus(newStatus);
    onAlarmOn();
  }
  else {
    if (MODE_ALARM == _currentMode) onAlarmOff(); //only once on alarm clean
    _setAlarmStatus(ALARM_OK); //always OK status if no alarm
  }

  _currentMode = newMode;

  //feeder
  setFeed(LOW); 
  tFeed.restart();

  //fan
  setFan(LOW);
  tFan.restart();

  //ignition timer restart
  setIgnition(false);
  if (MODE_IGNITION == newMode) tIgniter.restart();
  else tIgniter.disable();
  
  //min temperature should not go down in heat mode
  _intMinTemp = (MODE_HEAT == newMode) ? _intCurrentTemp - cfg.getMaxDropTemp() : 0; 

}

//http://bildr.org/2012/03/rfp30n06le-arduino/
void PitsBurner::setFan(byte percent) {
  burner._intFan = (byte)((float(percent) / 100.0) * 255.0);
  analogWrite(_pinFan, burner._intFan); 
  #ifdef _BURNER_SET_DEBUG_SERIAL_
    Serial.print(F("Fan is at: "));
    Serial.print(percent);
    Serial.print(F("% = PWM "));
    Serial.println(_intFan);
  #endif
}

byte PitsBurner::getFan() {
  return byte(float(_intFan) / 255 * 100);
}

bool PitsBurner::isFan() {
  return _intFan != LOW;
}


void PitsBurner::onFan() {
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.print(F("OnFan: "));
  #endif
  
  unsigned int interval;
  byte percent;
  bool change = true;
  
  switch (burner._currentMode) {
    
    case MODE_MANUAL:
      #ifdef _BURNER_DEBUG_SERIAL_
        Serial.print(F(" ignoring event in MODE_MANUAL, timer sleep 15s."));
      #endif
      tFan.setInterval(15*TASK_SECOND);
      change = false;
      break;
      
    case MODE_IGNITION:
      #ifdef _BURNER_DEBUG_SERIAL_
        Serial.print(F("in IGNITION"));
      #endif
      interval = cfg.getFeedIgnitionDelayS() * TASK_SECOND + cfg.getFeedIgnitionWorkS() * TASK_SECOND; //sync to feed time
      percent = cfg.getFanIgnitionOnP();
      break;
      
    case MODE_HEAT:
      #ifdef _BURNER_DEBUG_SERIAL_
      Serial.print(F("in HEAT"));
      #endif
      interval = (cfg.getFeedHeatWorkS() + cfg.getFeedHeatDelayS()) * TASK_SECOND; //sync to feed time
      percent = cfg.getFanHeatP(); //work all time
      break;
      
    case MODE_IDLE:
      #ifdef _BURNER_DEBUG_SERIAL_
        Serial.print(F("in IDLE"));
      #endif
      if (burner.isFan()) { 
        interval = cfg.getFeedIdleDelayS() * TASK_SECOND; //sync to feed time
        percent = cfg.getFanIdleOffP(); //LOW;
      }
      else {
        interval = cfg.getFeedIdleWorkS() * TASK_SECOND; //sync to feed time
        percent = cfg.getFanIdleOnP();
      }
      break;

    case MODE_CLEANING:
        interval = 3 * TASK_SECOND; //recheck burner status after 
        percent = cfg.getFanCleanP();
      
      
    default: //in OTHER mode turn feed off
      #ifdef _BURNER_DEBUG_SERIAL_
        Serial.print(F("in OTHER  "));
      #endif
      interval = 60 * TASK_SECOND; 
      percent = LOW;
      break;
  }

  tFan.setInterval(interval);
  if (change) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.print(F(" turn at "));
      Serial.print(percent);
      Serial.print(F("% during "));
      Serial.print(interval / TASK_SECOND);
      Serial.print(F("s "));
    #endif
    burner.setFan(percent);
  }
  
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.println();
  #endif
}

void PitsBurner::setFeed(byte percent) {
  _intFeeder = byte((float(percent) / 100) * 255);
  analogWrite(_pinFeeder, _intFeeder); 
  #ifdef _BURNER_SET_DEBUG_SERIAL_
    Serial.print(F("Feed is at: "));
    Serial.print(percent);
    Serial.print(F("% = PWM "));
    Serial.println(_intFeeder);
  #endif
}

byte PitsBurner::getFeed() {
  return byte(float(_intFeeder ) / 255 * 100);
}

bool PitsBurner::isFeed() {
  return _intFeeder != LOW;
}

void PitsBurner::onFeed() {
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.print(F("OnFeed: "));
  #endif
  
  int interval;
  byte feed;
  bool change = true;
  
  switch (burner._currentMode) {
    case MODE_MANUAL:
      #ifdef _BURNER_DEBUG_SERIAL_
        Serial.print(F(" ignoring event in MODE_MANUAL, timer sleep 15s."));
      #endif
      tFeed.setInterval(15*TASK_SECOND);
      change = false;
      break;
      
    case MODE_IGNITION:
      #ifdef _BURNER_DEBUG_SERIAL_
        Serial.print(F("in IGNITION "));
      #endif
      if (burner.isFeed()) {
        interval = cfg.getFeedIgnitionDelayS() * TASK_SECOND;
        feed = LOW;
      }
      else {
        interval = cfg.getFeedIgnitionWorkS() * TASK_SECOND;
        feed = cfg.getFeedIgnitionP();
      }
      break;
      
    case MODE_HEAT:
      #ifdef _BURNER_DEBUG_SERIAL_
        Serial.print(F("in HEAT "));
      #endif
      if (burner.isFeed()) {
        interval = cfg.getFeedHeatDelayS() * TASK_SECOND;
        feed = LOW;
      }
      else {
        interval = cfg.getFeedHeatWorkS() * TASK_SECOND;
        feed = cfg.getFeedHeatP();
      }
      break;
      
    case MODE_IDLE:
      #ifdef _BURNER_DEBUG_SERIAL_
        Serial.print(F("in IDLE "));
      #endif
      if (burner.isFeed()) {
        interval = cfg.getFeedIdleDelayS() * TASK_SECOND;
        feed = LOW;
      }
      else {
        interval = cfg.getFeedIdleWorkS() * TASK_SECOND;
        feed = cfg.getFeedIdleP();
      }
      break;
      
    default: //in OTHER modes no feed at all
      #ifdef _BURNER_DEBUG_SERIAL_
        Serial.print(F("in OTHER  "));
      #endif
      interval = 60 * TASK_SECOND; 
      feed = LOW;
      break;
  }
  
  tFeed.setInterval(interval);
  
  if (change) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.print(F("feed at "));
      Serial.print(feed);
      Serial.print(F("% during "));
      Serial.print(interval / TASK_SECOND);
      Serial.print(F("s "));
    #endif

    burner.setFeed(feed);

    //log total feed seconds
    if (feed != LOW) {
      burner._intFeedTime += (interval / TASK_SECOND);
    }
  }

  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.println();
  #endif  
}

byte PitsBurner::getFeedTime() {
  return _intFeedTime;
}

byte PitsBurner::getBattLevel() 
{
  return _intBattLevel;
}

void PitsBurner::setBattLevel(byte t) {
    #ifdef _BURNER_SET_DEBUG_SERIAL_
      Serial.print(F("Battery: "));
      Serial.println(t);
    #endif
  _intBattLevel = t;
}

byte PitsBurner::getFuelLevel() 
{
  return _intFuelLevel;
}

void PitsBurner::setFuelLevel(byte t) {
    #ifdef _BURNER_SET_DEBUG_SERIAL_
      Serial.print(F("Fuel: "));
      Serial.println(t);
    #endif
  _intFuelLevel = t;
}

bool PitsBurner::isPump()
{
  return _boolPump;
}

void PitsBurner::setPump(bool v) 
{
  if (_boolPump == v) return;
   
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.print(F("Pump "));
    Serial.println(v ? F("ON") : F("OFF"));
  #endif
  digitalWrite(_pinPump, v ? HIGH : LOW);
  _boolPump = v;
}

void PitsBurner::onIgnite() {

  if (MODE_IGNITION == burner._currentMode) {
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.print(F("OnIgnite: "));
    #endif
    
    int interval = 0;
    bool ignite = false;

    interval = burner.isIgnition() ? cfg.getIgniterDelayS() : 
                        (tIgniter.isFirstIteration() ? cfg.getIgniterStartS() : cfg.getIgniterWorkS());
    interval = interval * TASK_SECOND;
    ignite = !burner.isIgnition(); 

    tIgniter.setInterval(interval);
    #ifdef _BURNER_DEBUG_SERIAL_
      Serial.print(F("during "));
      Serial.print(interval / TASK_SECOND);
      Serial.print(F("s "));
    #endif
    
    burner.setIgnition(ignite);  
  }

}

void PitsBurner::setIgnition(bool turnOn) {
  _boolIgnition = turnOn;
  digitalWrite(_pinIgniter, turnOn ? HIGH : LOW); 

  #ifdef _BURNER_DEBUG_SERIAL_
    if (turnOn) Serial.println(F("Ignition ON"));
    else Serial.println(F("Ignition OFF"));
  #endif
}

bool PitsBurner::isIgnition() {
  return _boolIgnition;
}

void PitsBurner::onAlarmOn() {
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.println(F("Alarm ON"));
  #endif
  digitalWrite(_pinAlarm, HIGH);
}

void PitsBurner::onAlarmOff() {
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.println(F("Alarm OFF"));
  #endif
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

/*
Returns CM readings from ultrasonic sensor.
Reffer: http://arduino-project.net/podklyuchenie-ul-trazvukovogo-dal-nomera-hc-sr04-k-arduino/ 

 */
byte PitsBurner::_HYSRF05() {

  digitalWrite(_pinFuelTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(_pinFuelTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(_pinFuelTrig, LOW);

  short duration = pulseIn(_pinFuelEcho, HIGH, _msFuelTimout);
  //Serial.println(duration);

  if (duration == 0) duration = _msFuelTimout; //set to max if no reading
  
  return duration / 29 / 2; //return centemeters
}



