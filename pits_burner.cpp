#include "pits_burner.h"
PitsBurner burner;

extern BurnerConfig cfg;

#include "burner_ethernet.h"
extern BurnerEthernet net;

#include <TaskScheduler.h>
extern Scheduler scheduler;

Task tFeed(5*TASK_SECOND, TASK_FOREVER, &PitsBurner::onFeed, &scheduler, true);
Task tFan(5*TASK_SECOND, TASK_FOREVER, &PitsBurner::onFan, &scheduler, true);
Task tIgniter(1*TASK_SECOND, TASK_FOREVER, &PitsBurner::onIgnite, &scheduler, false);

void logme(const __FlashStringHelper *fmt, ... ){
  char buf[256]; 
  va_list args;
  va_start (args, fmt);
  #ifdef __AVR__
    vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
  #else
    vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
  #endif
  va_end(args);

  Serial.println(buf);
  net.logme(buf);
}

void PitsBurner::init() {
  
  pinMode(_pinTBoiler,INPUT);  
  pinMode(_pinTExhaust,INPUT);  
  pinMode(_pinTFeeder,INPUT);  
  pinMode(_pinFlameSensor,INPUT);  
  //pinMode(_pinLambda,INPUT);  
  pinMode(_pinBattery,INPUT);  
  pinMode(_pinFeedAmps,INPUT);
  pinMode(_pinFeedReverse,OUTPUT);
  
  pinMode(_pinAlarm,OUTPUT); 
  pinMode(_pinIgniter,OUTPUT);
  pinMode(_pinBuzzer,OUTPUT);
  pinMode(_pinFan,OUTPUT);  
  pinMode(_pinFeeder,INPUT);  
  pinMode(_pinFuelTrig,OUTPUT);
  pinMode(_pinFuelEcho,INPUT);
  pinMode(_pinPumpUPS,OUTPUT);

  //check buzzer
  beep();

  setCurrentMode(MODE_IDLE, ALARM_OK);
}

void PitsBurner::beep() {
  tone(_pinBuzzer, 1915);
  delay(150);
  noTone(_pinBuzzer);
}


void PitsBurner::operate() {
  _readSensors();
  _switchMode();
  _inAlarmOperate();
}

void PitsBurner::_readSensors() {

  setCurrentTemp((byte)_KTY81_110(_pinTBoiler));
  setExhaustTemp((byte)_KTY81_110(_pinTExhaust));
  setFeederTemp((byte)_KTY81_110(_pinTFeeder));
  setFlame((byte)_LDR04(_pinFlameSensor));
  updateLastFlameStatus();
  setFeederAmps(_ACS712(_pinFeedAmps) * 10); //convert to dcAmps

  //fuel level reading 
  _intFuelCm = _HYSRF05(_pinFuelTrig, _pinFuelEcho);
  if (_intFuelCm > cfg.getFuelLevel(P0)) setFuelLevel(P0);
  else if (_intFuelCm > cfg.getFuelLevel(P20)) setFuelLevel(P20);
  else if (_intFuelCm > cfg.getFuelLevel(P40)) setFuelLevel(P40);
  else if (_intFuelCm > cfg.getFuelLevel(P60)) setFuelLevel(P60);
  else if (_intFuelCm > cfg.getFuelLevel(P80)) setFuelLevel(P80);
  else setFuelLevel(P100);

  //battery level reading
  _intBattDVolts = (byte)roundf(_vDivVin(5500, 2200, _pinBattery)*10); //R2 actually is 2200ohm, but calibrated value is different.  
  _intBattDVolts += 10; //diode 1N1007 in 12 input reduces by 10dcV
  if (_intBattDVolts >= cfg.getBattLevel(P100)) setBattLevel(CHARGE);
  else if (_intBattDVolts >= cfg.getBattLevel(P80)) setBattLevel(P100);
  else if (_intBattDVolts >= cfg.getBattLevel(P60)) setBattLevel(P80);
  else if (_intBattDVolts >= cfg.getBattLevel(P40)) setBattLevel(P60);
  else if (_intBattDVolts >= cfg.getBattLevel(P20)) setBattLevel(P40);
  else if (_intBattDVolts >= cfg.getBattLevel(P0)) setBattLevel(P20);
  else setBattLevel(P0);

  logme(F("&ms=%d&M=%d&CurT=%d&ExhT=%d&Flm=%d&FlmReq=%d&NoFlm=%d&FeedT=%d&FeedA=%d&Fuel=%d&FuelCm=%d&Bat=%d&BatV=%d&Alm=%d"), 
    getSecondsInCurrentMode(), burner.getCurrentMode(),
    getCurrentTemp(), 
    getExhaustTemp(), 
    getFlame(), cfg.getFlameLevel(), getSecondsWithoutFlame(),
    getFeederTemp(), getFeederAmps(), 
    getFuelLevel(), _intFuelCm, 
    getBattLevel(), _intBattDVolts, 
    burner.getAlarmStatus()
  );

/*                                                                                                                                          -
  logme(F("MdS=%d&Mode=%d&CurT=%d&CurTV=%d&ReqT=%d&ExhT=%d&ExhTV=%d&Flm=%d&FlmV=%d&FlmReq=%d&NoFlm=%d&FeedT=%d&FeedTV=%d&Fuel=%d&FuelCm=%d&Batt=%d&BattDcV=%d&BattV=%d&FeedA=%d&FeedAV=%d&Alarm=%d"), 
    getSecondsInCurrentMode(), burner.getCurrentMode(),
    getCurrentTemp(), analogRead(_pinTBoiler), cfg.getRequiredTemp(),
    getExhaustTemp(), analogRead(_pinTExhaust),
    getFlame(), analogRead(_pinFlameSensor), cfg.getFlameLevel(), getSecondsWithoutFlame(),
    getFeederTemp(), analogRead(_pinTFeeder),
    getFuelLevel(), _intFuelCm, 
    getBattLevel(), _intBattDVolts, analogRead(_pinBattery),
    getFeederAmps(), analogRead(_pinFeedAmps),
    burner.getAlarmStatus()
  );
*/
}

byte PitsBurner::getMinTemp() {
  return _intMinTemp;
}

void PitsBurner::_switchMode() {

  //do nothing in MODE_MANUAL
  if (getCurrentMode() == MODE_MANUAL) return;

  //control circulation pumps UPS
  if (getBattLevel() != CHARGE && getCurrentTemp() > cfg.getPumpOnTemp() + cfg.getHysteresisTemp() && isPumpUPS() == false) {
    logme(F("INFO-PumpUPS On & CurT=%d"), getCurrentTemp());
    setPumpUPS(true);
  }
  else if (getCurrentTemp() < _intMinTemp - cfg.getHysteresisTemp() && isPumpUPS() == true) {
    logme(F("INFO-PumpUPS Off & CurT="), getCurrentTemp());
    setPumpUPS(false);
  }

  //control feed current and reverse if overload
  if (isFeedReverse() && getFeederAmps() > cfg.getFeedAmpsMax()) { //stuck in both directions
    logme(F("ALARM-Feed-stuck& Amps=%d"), getFeederAmps());
    setCurrentMode(MODE_ALARM, ALARM_FEED_STUCK);
  }
  else if (getFeederAmps() > cfg.getFeedAmpsRev()) { //stcuk - reverse ON
    logme(F("WARNING-Reverse-feed&Amps=%d"), getFeederAmps());
    setFeedReverse(true);
  }
  else { //continue normal direction
    setFeedReverse(false);
  }

  //if temperature lower then minimum during heat cycle
  //NOTE1: when main circulation turns on we expect temperature drop up to 10 degree
  //NOTE2: _intMinTemp set to "cur temp" - "alarm drop temp" in the begining of heat cycle
  if ( getCurrentTemp() < getMinTemp() && getCurrentMode() == MODE_HEAT) {
    logme(F("ALARM-Underheat&CurT=%d&MinT=%d"), getCurrentTemp() , getMinTemp());
    setCurrentMode(MODE_ALARM, ALARM_TEMPDROP);
    return;
  }

  //if overheating then alarm
  if (getCurrentTemp() > cfg.getMaxTemp() && getCurrentMode() != MODE_ALARM) {
    logme(F("ALARM-overheat&CurT=%d&MaxT=%d"), getCurrentTemp() ,cfg.getMaxTemp());
    setCurrentMode(MODE_ALARM, ALARM_OVERHEAT);
    return;
  }  

  //if burner temp overheat then alarm
  //TODO: add to alarm display screen feeder temp alarm
  /*
  if (getFeederTemp() != 0 && getFeederTemp() > 80 && getCurrentMode() != MODE_ALARM) {
    logme(F("ALARM-overheat feeder & FedT=%d"), getFeederTemp());
    setCurrentMode(MODE_ALARM, ALARM_OVERHEAT_FEED);
    return;
  }
  */

  //if no flame 3 minutes then switch to ignite
  //TODO: do not feed on switch to ignition - there already a lot of pellets to ignite.
  if (getCurrentMode() == MODE_HEAT && getSecondsWithoutFlame() > cfg.getFlameTimoutS()) {
    logme(F("IGNITION-no-flame&Sec=%d&Tout=%d"), getSecondsWithoutFlame(), cfg.getFlameTimoutS());
    resetFlameTimer();
    setCurrentMode(MODE_IGNITION, ALARM_NOFLAME);
    return;
  }

  //if ignition and no flame for X minutes then alarm
  if (getCurrentMode() == MODE_IGNITION && getSecondsWithoutFlame() > 15*60) {
    logme(F("ALARM-ignition-timeout&Sec=%d"), getSecondsWithoutFlame());
    setCurrentMode(MODE_ALARM, ALARM_IGNITION_FAILED);
    return;
  }

  //if exhaust sensor exists, if exhaust temperature more than required temperature + allowed delta then idle
  if (getCurrentMode() == MODE_HEAT && getExhaustTemp() > 0 && getExhaustTemp()  > (cfg.getRequiredTemp() + cfg.getExhaustDeltaTemp()) ) {
    logme(F("IDLE-Exhaust-limit&ExhT=%d&ReqT=%d&ExhDT=%d"), getExhaustTemp(), cfg.getRequiredTemp(), cfg.getExhaustDeltaTemp());
    setCurrentMode(MODE_IDLE, ALARM_OK);
    return;
  }

  //TODO: if in idle mode longer than X hours then alarm (do we know use case for this?)

  //if heating and reached requied temperature then clean and then idle
  if (getCurrentMode() == MODE_HEAT && getCurrentTemp() > (cfg.getRequiredTemp() + cfg.getHysteresisTemp()) ) {
    logme(F("CLEAN-Temp-reached&CurT=%d&ReqT=%d&Hist=%d"), getCurrentTemp(), cfg.getRequiredTemp(), cfg.getHysteresisTemp() );
    setCurrentMode(MODE_CLEANING, ALARM_OK);
    return;
  }
  else if (getCurrentMode() == MODE_CLEANING && getSecondsInCurrentMode() > cfg.getFanCleanWorkS() ) {
    logme(F("IDLE-Temp-reached&ModeS=%d&FanCleanS=%d"), getSecondsInCurrentMode(),  cfg.getFanCleanWorkS() );
    setCurrentMode(MODE_IDLE, ALARM_OK);
    return;
  }

  //if ignition and is flame then switch to heat 
  if (getCurrentMode() == MODE_IGNITION && isFlame()) {
    logme(F("HEAT-See-flame&Flame=%d"), getFlame());
    setCurrentMode(MODE_HEAT, ALARM_OK);
    return;
  }

  //if under required temperature then heat
  if ( (getCurrentMode() == MODE_IDLE && getCurrentTemp() < (cfg.getRequiredTemp()  - cfg.getHysteresisTemp())) &&
       (getExhaustTemp() == 0 || getExhaustTemp()  <= cfg.getRequiredTemp()) &&
       isCurrentModeStable() ) {
    logme(F("HEAT-Under-temperature&CurT=%d&ReqT=%d&Hist=%d"),  getCurrentTemp(), cfg.getRequiredTemp(), cfg.getHysteresisTemp() );
    resetFlameTimer();
    setCurrentMode(MODE_HEAT, ALARM_OK);
    return;
  }

}

void PitsBurner::_inAlarmOperate() {
  
  //different sounds based on alarm code
  switch (_alarmStatus) {
    case ALARM_OK:
      break;
    case ALARM_MANUAL:
      tone(_pinBuzzer, 1915);
      delay(20);
      break;
    case ALARM_TEMPDROP:
      tone(_pinBuzzer, 1915);
      delay(50);
      tone(_pinBuzzer, 1700);
      delay(50);
      break;
    case ALARM_NOFLAME:
      tone(_pinBuzzer, 1915);
      delay(50);
      tone(_pinBuzzer, 1700);
      delay(50);
      break;
    case ALARM_IGNITION_FAILED:
      tone(_pinBuzzer, 1400);
      delay(100);
      break;
    case ALARM_OVERHEAT:
      tone(_pinBuzzer, 1915);
      delay(100);
      break;
  }
  noTone(_pinBuzzer);
}

void PitsBurner::_setAlarmStatus(PitsAlarmStatus newStatus) {
  //logme(String(F("Alarm Status:")) + newStatus);
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
    _intCurrentTemp = (_intCurrentTemp + t) / 2;
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
    _intExhaustTemp = (_intExhaustTemp + t) / 2; //floating average
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
  if (isFlame() || MODE_MANUAL == getCurrentMode()) {
    //Serial.println("Flame reset!");
    _uiTimeWithoutFlame = 0;
  } else if (_uiTimeWithoutFlame == 0) { //record when flame lost, once!
    _uiTimeWithoutFlame = roundf(millis() / 1000.0); 
    if (_uiTimeWithoutFlame == 0) _uiTimeWithoutFlame = 1; //on arduino start millis()=0
  }
}

short PitsBurner::getSecondsWithoutFlame() {
  if (_uiTimeWithoutFlame == 0) { //if flame
    return 0;
  }
  else { //if no flame
    short noFlameDelta = millis() / 1000 - _uiTimeWithoutFlame;
    //Serial.println(String("noFlameDelta: ") + noFlameDelta);
    return noFlameDelta;
  }
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

void PitsBurner::setFeederAmps(byte t) {
  #ifdef _BURNER_SET_DEBUG_SERIAL_
    Serial.print(F("FeederAmps: "));
    Serial.println(t);
  #endif
  _flFeedAmps[2] = _flFeedAmps[1];
  _flFeedAmps[1] = _flFeedAmps[0];
  _flFeedAmps[0] = t;
}

byte PitsBurner::getFeederAmps() {
  return (_flFeedAmps[0] + _flFeedAmps[1] + _flFeedAmps[2]) / 3;
}

void PitsBurner::setFeederTemp(byte t) {
  if (_intFeederTemp != t) {
    #ifdef _BURNER_SET_DEBUG_SERIAL_
      Serial.print(F("FeederTemp: "));
      Serial.print(_intFeederTemp);
      Serial.print(F("->"));
      Serial.println(t);
    #endif
    _intFeederTemp = (_intFeederTemp + t) / 2;
  }
}

byte PitsBurner::getFeederTemp() {
  return _intFeederTemp;
}

void PitsBurner::resetSecondsInCurrentMode()
{
  #ifdef _BURNER_SET_DEBUG_SERIAL_
    Serial.print(F("Reset seconds in current mode to "));
    Serial.println(_uiModeTimer);
  #endif
  _uiModeTimer = roundf(millis() / 1000.0);
  if (_uiModeTimer == 0) _uiModeTimer = 1; //on arduino start millis()=0
}

short PitsBurner::getSecondsInCurrentMode() {
  if (_uiModeTimer == 0) { //not set
    return 0;
  }
  else { //set
    return millis() / 1000.0 - _uiModeTimer;
  }
}

bool PitsBurner::isCurrentModeStable() 
{
  return getSecondsInCurrentMode() > _uiStableMode;
}


PitsBurnerMode PitsBurner::getCurrentMode() {
  return _currentMode;
}

void PitsBurner::setCurrentMode(PitsBurnerMode newMode, PitsAlarmStatus newStatus) {
  #ifdef _BURNER_SET_DEBUG_SERIAL_
    Serial.print(F("Mode change: "));
    Serial.println(newMode);
  #endif

  //do nothing if this is same 
  if (newMode == _currentMode) return;
  
  //reset mode duration timer
  resetSecondsInCurrentMode();
  
  if (MODE_MANUAL == newMode) {
    onAlarmOff(); 
    _setAlarmStatus(newStatus);
    setPumpUPS(false);
    setFeedReverse(false);
    setFeed(LOW); 
    tFeed.disable();
    setFan(LOW);
    tFan.disable();
    setIgnition(false);
    tIgniter.disable();
    resetFlameTimer();
    _intMinTemp = 0;
    _currentMode = newMode;
    logme(F("MANUAL-mode"));
  }
  else {

    //trigger alarm events
    if (MODE_ALARM == newMode) {
      _setAlarmStatus(newStatus);
      onAlarmOn();
    }
    else {
      if (MODE_ALARM == _currentMode) onAlarmOff(); //only once on alarm clean
      _setAlarmStatus(ALARM_OK);
    }
  
    _currentMode = newMode; //location is important

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
    _intMinTemp = (MODE_HEAT == newMode && _intCurrentTemp > cfg.getMaxDropTemp()) ? _intCurrentTemp - cfg.getMaxDropTemp() : 0; 

  }
}

//Setup theme - http://bildr.org/2012/03/rfp30n06le-arduino/
//Fan model - http://www.nmbtc.com/pdf/catalogs/Fan_and_Blowers_Catalog_Full.pdf  
void PitsBurner::setFan(byte percent) {
  burner._intFan = (byte)roundf(float(percent) * 255.0 / 100);
  analogWrite(_pinFan, burner._intFan); 
  #ifdef _BURNER_SET_DEBUG_SERIAL_
    Serial.print(F("Fan is at: "));
    Serial.print(percent);
    Serial.print(F("% = PWM "));
    Serial.println(_intFan);
  #endif
}

byte PitsBurner::getFan() {
  return (byte)roundf(float(_intFan) * 100 / 255);
}

bool PitsBurner::isFan() {
  return _intFan != LOW;
}


void PitsBurner::onFan() {
  unsigned long interval;
  byte percent;
  bool change = true;
  
  switch (burner.getCurrentMode()) {
    
    case MODE_MANUAL:
      tFan.setInterval(15*TASK_SECOND);
      change = false;
      break;
      
    case MODE_IGNITION:
      if (burner.getFan() == cfg.getFanIgnitionOnP()) { //if HIGH in idle
        interval = (cfg.getFeedIgnitionWorkS() + cfg.getFeedIgnitionDelayS() - cfg.getFanIgnitionWorkS()) * TASK_SECOND; //sync to feed time
        if (interval < 0) interval = 1 * TASK_SECOND; //fix if user configuration error
        percent = cfg.getFanIgnitionOffP(); //set to LOW
      }
      else {
        interval = cfg.getFanIgnitionWorkS() * TASK_SECOND; 
        percent = cfg.getFanIgnitionOnP(); //set to HIGH
      }
      break;
      
    case MODE_HEAT:
      interval = (cfg.getFeedHeatWorkS() + cfg.getFeedHeatDelayS()) * TASK_SECOND; //sync to feed time
      percent = cfg.getFanHeatP(); //work all time
      break;
      
    case MODE_IDLE:
      if (burner.getFan() == cfg.getFanIdleOnP()) { //if HIGH in idle
        interval = (cfg.getFeedIdleWorkS() + cfg.getFeedIdleDelayS() - cfg.getFanIdleWorkS()) * TASK_SECOND; //sync to feed time
        if (interval < 0) interval = 1 * TASK_SECOND; //fix if user configuration error
        percent = cfg.getFanIdleOffP(); //set to LOW
      }
      else {
        interval = cfg.getFanIdleWorkS() * TASK_SECOND; 
        percent = cfg.getFanIdleOnP(); //set to HIGH
      }
      break;

    case MODE_CLEANING:
      interval = cfg.getFanCleanWorkS() * TASK_SECOND;
      percent = cfg.getFanCleanP();
      break;
      
    case MODE_ALARM: 
      interval = 30 * TASK_SECOND; 
      //TODO: get calues from config and add values to display configuration
      percent = burner.getFeederTemp() > 35 ? cfg.getFanIdleOffP() : 0; //keep fan ON until temp drops 
      break;

    default: //in OTHER mode turn feed off
      interval = 60 * TASK_SECOND; 
      percent = LOW;
      break;
  }

  tFan.setInterval(interval);
  if (change) {
    burner.setFan(percent);
  }
  logme(F("Fan&FnPw=%d&FnS=%d&Chg=%d"), percent, interval / TASK_SECOND, change);
  
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.println();
  #endif
}

void PitsBurner::setFeed(byte percent) {
  _intFeeder = (byte)roundf(float(percent) * 255 / 100);
  analogWrite(_pinFeeder, _intFeeder); 
  #ifdef _BURNER_SET_DEBUG_SERIAL_
    Serial.print(F("Feed is at: "));
    Serial.print(percent);
    Serial.print(F("% = PWM "));
    Serial.println(_intFeeder);
  #endif
}

byte PitsBurner::getFeed() {
  return (byte)roundf(float(_intFeeder ) * 100 / 255 );
}

bool PitsBurner::isFeed() {
  return _intFeeder != LOW;
}

void PitsBurner::onFeed() {
  unsigned long interval; //max value could be 200k in delay
  byte percent;
  bool change = true;
  
  switch (burner.getCurrentMode()) {
    case MODE_MANUAL:
      tFeed.setInterval(15*TASK_SECOND);
      change = false;
      break;
      
    case MODE_IGNITION:
      if (burner.isFeed() || !burner.isCurrentModeStable()) { //skip feed on first seconds in mode
        interval = cfg.getFeedIgnitionDelayS() * TASK_SECOND;
        percent = LOW;
      }
      else {
        interval = cfg.getFeedIgnitionWorkS() * TASK_SECOND;
        percent = cfg.getFeedIgnitionP();
      }
      break;
      
    case MODE_HEAT:
      if (burner.isFeed() || !burner.isCurrentModeStable()) {
        interval = cfg.getFeedHeatDelayS() * TASK_SECOND;
        percent = LOW;
      }
      else {
        interval = cfg.getFeedHeatWorkS() * TASK_SECOND;
        percent = cfg.getFeedHeatP();
      }
      break;
      
    case MODE_IDLE:
      if (burner.isFeed() || !burner.isCurrentModeStable()) { //skip feed on first seconds in mode
        interval = cfg.getFeedIdleDelayS() * TASK_SECOND;
        percent = LOW;
      }
      else {
        interval = cfg.getFeedIdleWorkS() * TASK_SECOND;
        percent = cfg.getFeedIdleP();
      }
      break;
      
    default: //in OTHER modes no feed at all
      interval = 30 * TASK_SECOND; 
      percent = LOW;
      break;
  }
  
  tFeed.setInterval(interval);
  
  if (change) {
    burner.setFeed(percent);

    //log total feed seconds
    if (percent != LOW) {
      burner._intFeedTime += (interval / TASK_SECOND);
    }
  }

  logme(F("Feed&FdP=%d&FdS=%d&Mn=%d&TotFdS=%d"), percent, interval / TASK_SECOND, change, burner._intFeedTime);
}

byte PitsBurner::getFeedTime() {
  return _intFeedTime;
}

PERCENT_RANGE PitsBurner::getBattLevel() 
{
  return _intBattLevel;
}

byte PitsBurner::getBattDVolts() 
{
  return _intBattDVolts;
}

void PitsBurner::setBattLevel(PERCENT_RANGE t) {
    #ifdef _BURNER_SET_DEBUG_SERIAL_
      Serial.print(F("Battery: "));
      Serial.println(t);
    #endif
  _intBattLevel = t;
}

PERCENT_RANGE PitsBurner::getFuelLevel() 
{
  return _intFuelLevel;
}

byte PitsBurner::getFuelCm() 
{
  return _intFuelCm;
}

void PitsBurner::setFuelLevel(PERCENT_RANGE t) {
    #ifdef _BURNER_SET_DEBUG_SERIAL_
      Serial.print(F("Fuel: "));
      Serial.println(t);
    #endif
  _intFuelLevel = t;
}

bool PitsBurner::isFeedReverse()
{
  return _boolFeedRev;
}

void PitsBurner::setFeedReverse(bool v) 
{
  if (_boolFeedRev == v) return;
  
  #ifdef _BURNER_DEBUG_SERIAL_
    Serial.print(F("Feed reverse "));
    Serial.println(v ? F("ON") : F("OFF"));
  #endif

  digitalWrite(_pinFeedReverse, v ? HIGH : LOW);
  _boolFeedRev= v;
}


bool PitsBurner::isPumpUPS()
{
  return _boolPumpUPS;
}

void PitsBurner::setPumpUPS(bool v) 
{
  if (_boolPumpUPS == v) return;
   
  #ifdef _BURNER_SET_DEBUG_SERIAL_
    Serial.print(F("PumpUPS "));
    Serial.println(v ? F("ON") : F("OFF"));
  #endif
  digitalWrite(_pinPumpUPS, v ? HIGH : LOW);
  _boolPumpUPS = v;
}

void PitsBurner::onIgnite() {

  if (MODE_IGNITION == burner.getCurrentMode()) {
    int interval = 0;
    bool ignite = false;

    interval = burner.isIgnition() ? cfg.getIgniterDelayS() : 
                        (tIgniter.isFirstIteration() ? cfg.getIgniterStartS() : cfg.getIgniterWorkS());
    interval = interval * TASK_SECOND;
    ignite = !burner.isIgnition(); 

    tIgniter.setInterval(interval);
    burner.setIgnition(ignite);  

    logme(F("Ignite&IgnP=%d&IgnS=%d"), ignite, interval / TASK_SECOND);
  }

}

void PitsBurner::setIgnition(bool turnOn) {
  _boolIgnition = turnOn;
  digitalWrite(_pinIgniter, turnOn ? HIGH : LOW); 

  #ifdef _BURNER_SET_DEBUG_SERIAL_
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
Converts termistor KTY81-110 analog readings to celsius.
DataSheet - http://www.tme.eu/ru/Document/63412cca845bf05e8bcce2eecca1aa6d/KTY81-210.pdf
Codesample - http://electronics.stackexchange.com/questions/188813/strange-result-adcarduino-micro-thermistor-kty-10-6
Source - https://www.lemona.lv/?page=item&i_id=30742
*/                                
float PitsBurner::_KTY81_110(byte pin) {
    const int resistor = 2120; //charger 2600; //usb 1950; //2200 = 2K2 resistor, use resistors +/- 1% deviation 

  float ukty = 5.1 * analogRead(pin) / 1023.0 ;
  float a = 0.01874;
  float b = 7.884;
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
float PitsBurner::_LDR04(byte pin) {
  const int Res1 = 2200; // Resistor 1 value - 2k2 

  short sensorValue = analogRead(pin);
  float Vout1 = sensorValue * (5.0/1023.0);      // calculate Voltage 1 (one unit = 5v / 1024)
  float Res2 = ((5.0  * Res1) / Vout1) - Res1;   // caclulate  Resistor 2
  float retVal = (Res2 / (Res1 + Res2)) * 100;
  //Serial.println(String("_LDR04:") + retVal);
  return retVal;
}

/*
Returns CM readings from ultrasonic sensor.
Reffer: http://arduino-project.net/podklyuchenie-ul-trazvukovogo-dal-nomera-hc-sr04-k-arduino/ 
*/
byte PitsBurner::_HYSRF05(byte pinTrig, byte pinEcho) {

  const short intFuelTimout = 15000;     //// depends on max distance - 3000 µs = 50cm // 30000 µs = 5 m

  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  short duration = pulseIn(pinEcho, HIGH, intFuelTimout);
  //Serial.println(duration);

  if (duration == 0) duration = intFuelTimout; //set to max if no reading
  
  return duration / 29 / 2; //return centemeters
}

/*
Output voltage recalculation to input voltage
Reffer: http://www.ohmslawcalculator.com/voltage-divider-calculator
Use over voltage protetion: http://www.learningaboutelectronics.com/Articles/Overvoltage-protection-circuit.php
*/
float PitsBurner::_vDivVin(float R1, float R2, byte pin) {
  float vIn = 5.0 * analogRead(pin) / 1023.0 ;
  //Serial.println(String("vDivVin: vIn=") + vIn );
  return vIn/(R2/(R1+R2));
}

/*
  Returns  current consumption reading from ACS712 sensor.
  Motor max I (amps) = U (volts) / R (ohms), where U=12V and R is resistance of motor coil.
  Feeder - 10A max
  Fan 4mA - max
 */
float PitsBurner::_ACS712(byte pin) {
  const byte mVperAmp = 66; // use 185 for 10A, 100 for 20A Module and 66 for 30A Module
  const short ACSoffset = 2485; //milivolts 

  short rawVal = analogRead(pin);
  float mVolts = ((rawVal+1) / 1024.0) * 5000;
  float amps = ((mVolts - ACSoffset) / mVperAmp);

  //if (amps > 4) Serial.println(">>>>>>>>>>> HIGH!!!!");
  //Serial.println(String("ACS712 ") + pin + "=" + rawVal + " => " + mVolts + "mV, " + amps  + "A");
  return amps;
}


