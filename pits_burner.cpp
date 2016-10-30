#include "Arduino.h"
#include "pits_burner.h"
#include <math.h>

PitsBurner burner;

Task tOperate(1*TASK_SECOND, TASK_FOREVER, &PitsBurner::onOperate, &scheduler, true);
Task tFeed(1*TASK_SECOND, TASK_FOREVER, &PitsBurner::onFeed, &scheduler, true);
Task tFan(1*TASK_SECOND, TASK_FOREVER, &PitsBurner::onFan, &scheduler, true);

PitsBurner::PitsBurner() {
  return;
}

void PitsBurner::init(int pinTBoiler, int pinTExhaust, int pinFlameSensor, int pinTFeeder, int pinFan, int pinFeeder) {

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
  digitalWrite(_pinFan, LOW);
  Serial.print(", _pinFan=");
  Serial.print(_pinFan);
  
  _pinFeeder = pinFeeder;
  pinMode(_pinFeeder, OUTPUT);
  digitalWrite(_pinFeeder, HIGH);
  Serial.print(", pinFeeder=");
  Serial.print(_pinFeeder);
  
  Serial.println(". Initiated.");

}



void PitsBurner::onOperate() {

   
  burner._readSensors();

  burner._switchMode();

}

void PitsBurner::_readSensors() {

  setFlame(_LDR04(analogRead(_pinFlameSensor)));
  setFeederTemp(_KTY81_110(analogRead(_intFeederTemp)));
  setCurrentTemp(_KTY81_110(analogRead(_pinTBoiler)));
  setExhaustTemp(_KTY81_110(analogRead(_pinTExhaust)));
  Serial.println(String("PitsBurner - ReadSensors: CurrT=") + getCurrentTemp() + " ->" + getRequiredTemp() + ", Flame=" + getFlame() + ", ExhaT=" + getExhaustTemp() + ", FeedT=" + getFeederTemp());
}

void PitsBurner::_switchMode() {

  //do nothing if stopped
  if (_currentMode != MODE_STOP) return;

  //if temperature lower then minimum and not in ignition mode
  if (_intCurrentTemp < _intMinTemp && _currentMode != MODE_IGNITION && _currentMode != MODE_ALARM) {
    setCurrentMode(MODE_ALARM);
  }

  //if no flame then ignite

  //if ignition and no flame for 15 minutes then alarm

  //if under required temperature then heat
  if (_intCurrentTemp - _intRequiredTemp < -_intHysteresisTemp && _currentMode == MODE_IDLE) {
    setCurrentMode(MODE_HEAT);
  }


  //if exhaust temperature more then  boiler temperature + allowed delta then idle
  //option: if exhaust sensor exists
  if (_intExhaustTemp > 0 && _intExhaustTemp - _intCurrentTemp > _intExhDeltaTemp && _currentMode == MODE_HEAT) {
    setCurrentMode(MODE_IDLE);
  }

  //if under equired temperature during 1 hour then alarm


  //if heating and reached requied temperature then idle
  if (_intCurrentTemp - _intRequiredTemp > _intHysteresisTemp && _currentMode == MODE_HEAT) {
    setCurrentMode(MODE_IDLE);
  }

  //if in idle mode longer than 1 hour then alarm
  
  //if overheating then alarm
  if (_intCurrentTemp > _intMaxTemp && _currentMode != MODE_ALARM) {
    setCurrentMode(MODE_ALARM);
  }

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
  Serial.println(String("PitsBurner - Flame: ") + _intFlame + " to " + t);
  _intFlame = t;
}

int PitsBurner::getFlame() {
  return _intFlame;
}

bool PitsBurner::isFlame() {
  return _intFlame > 5; 
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

bool PitsBurner::setCurrentMode(PitsBurnerMode mode) {
  if (_currentMode != mode) {
    Serial.println(String("PitsBurner - Mode: ") + _currentMode + " to " + mode);
    _currentMode = mode;
    _isFeed = false;
    _intFan = 0;
    tFeed.restart();
    tFan.restart();
  }
}

//http://bildr.org/2012/03/rfp30n06le-arduino/
void PitsBurner::setFan(int percent) {
  _intFan = int((float(percent) / 100) * 255);
  analogWrite(_pinFan, burner._intFan); 
  Serial.println("PitsBurner: Fan is at " + String(percent) + "% = " + String(_intFan) + " PWM");
}

int PitsBurner::getFan() {
  return int(float(_intFan) / 255 * 100);
}

void PitsBurner::onFan() {
  Serial.print("PitsBurner - OnFan: ");

  long intrval;
  int percent;
  
  switch (burner._currentMode) {
    case MODE_IGNITION:
      intrval = (burner._intFeedDelayIgnitionS + burner._intFeedTimeIgnitionS) * TASK_SECOND;
      percent = burner._intFanIgnitionP;
      Serial.print("in IGNITION ");
      break;
    case MODE_HEAT:
      intrval = (burner._intFeedDelayHeatS + burner._intFeedTimeHeatS) * TASK_SECOND;
      percent = burner._intFanHeatP;
      Serial.print("in HEAT ");
      break;
    case MODE_IDLE:
      if (burner._isFeed) {
        intrval = burner._intFanIdleWorkS * TASK_SECOND;
        percent = burner._intFanIdleP;
      }
      else {
        intrval = (burner._intFeedTimeIdelS  + burner._intFeedDelayIdleS - burner._intFanIdleWorkS) * TASK_SECOND;
        percent = 0;
      }
      break;
    default: //in other modes no feed at all
      intrval = 60 * TASK_SECOND; 
      percent = 0;
      break;
  }

  Serial.print("during " + String(intrval / TASK_SECOND) + " seconds ");
  tFan.setInterval(intrval);
  
  burner.setFan(percent);
}

void PitsBurner::setFeed(bool f) {
  _isFeed = f;
  digitalWrite(_pinFeeder, _isFeed ? HIGH : LOW);
  Serial.println("Feeder is " + String(_isFeed ? "ON" : "OFF"));
}

bool PitsBurner::getFeed() {
  return _isFeed;
}

void PitsBurner::onFeed() {
  Serial.print("PitsBurner - OnFeed: ");

  long interval;
  bool feed;
  
  switch (burner._currentMode) {
    case MODE_IGNITION:
      interval = burner.getFeed() ? burner._intFeedDelayIgnitionS * TASK_SECOND : burner._intFeedTimeIgnitionS * TASK_SECOND;
      feed = !burner.getFeed();
      Serial.print("in IGNITION ");
      break;
    case MODE_HEAT:
      interval = burner.getFeed() ? burner._intFeedDelayHeatS * TASK_SECOND : burner._intFeedTimeHeatS * TASK_SECOND;
      feed = !burner.getFeed();
      Serial.print("in HEAT ");
      break;
    case MODE_IDLE:
      interval = burner.getFeed() ? burner._intFeedDelayIdleS * TASK_SECOND : burner._intFeedTimeIdelS * TASK_SECOND;
      feed = !burner.getFeed();
      Serial.print("in IDLE ");
      break;
    default: //in other modes no feed at all
      interval = 60* TASK_SECOND; 
      feed = false;
      break;
  }
  
  Serial.print("during " + String(interval / TASK_SECOND) + " seconds ");
  tFeed.setInterval(interval);
  
  burner.setFeed(feed);
}

/*
Converts termistor KTY81-110 analog readings to celsius.
DataSheet - http://www.tme.eu/ru/Document/63412cca845bf05e8bcce2eecca1aa6d/KTY81-210.pdf
Codesample - http://electronics.stackexchange.com/questions/188813/strange-result-adcarduino-micro-thermistor-kty-10-6
*/                                
float PitsBurner::_KTY81_110(float sensorValue) {
  const int resistor = 970; //1k

  // calculate sensor resistance value (Rkty)
  float Rkty = (resistor * sensorValue)/(1023 - sensorValue);
  // From the data sheet the value of the resistance of the sensor @ 25 degrees is 1000 +/- 20 ohmsStart with calculating the measured resistance.
  float R25 = 990;
  //we are also given alpha and beta
  float alpha = 0.00788; //7.88/1000
  float beta = 0.0001937; //1.937/10000 
  //Now we need to calculate the temperature factor (KayTee)
  float KayTee = Rkty/R25 ;
  //We now have all the information to calculate the actual temperature (AcT)
  return 25 + ((sqrt((alpha*alpha)-(4*beta)+(4*beta*KayTee)) - alpha)/(2*beta)) ;
}

/*
Converts photo resistor LDR04 analog readings to 2-20K percent.
DataSheet - http://www.velleman.eu/products/view/?country=be&lang=en&id=167303
Codesample - https://arduinodiy.wordpress.com/2013/11/03/measuring-light-with-an-arduino/
*/
float PitsBurner::_LDR04(float sensorValue) {
  const int Res0 = 2200; //2k2
  float Vout0=sensorValue*0.0048828125;      // calculate the voltage (one unit = 5v / 1024)
  float lux = 500/(Res0*((5-Vout0)/Vout0));           // calculate the Lux
  //Serial.println(lux);
  return lux * 100;
}



