#include "nex_display.h"
NexDisplay nexdisp;

#include "pits_burner.h"
extern PitsBurner burner;

#include "burner_config.h"
extern BurnerConfig cfg;

static uint32_t _rv[15]; //packet of numbers read buffer
#define nexSerial Serial

void NexDisplay::init() {
  
    //nexSerial.begin(115200); //already set earlier
    __nxSendCommand("");
    __nxSendCommand("bkcmd=0"); //no return for send commands
    __nxSendCommand("page 0");

    __criticalSection = false;
    loadConfig();
    refresh();
}

void NexDisplay::loop() {
 
    static uint8_t __buffer[10];
    
    uint16_t i;
    uint8_t c;  
    
    __criticalSection = true;
    while (nexSerial.available() > 0)
    {   
      delay(10);
      c = nexSerial.read();
      Serial.print(c, HEX);
      
      if (NEX_RET_EVENT_TOUCH_HEAD == c)
      {
        if (nexSerial.available() >= 6)
        {
          __buffer[0] = c;  
          for (i = 1; i < 7; i++)
          {
              __buffer[i] = nexSerial.read();
          }
          __buffer[i] = 0x00;
          
          if (0xFF == __buffer[4] && 0xFF == __buffer[5] && 0xFF == __buffer[6])
          {
              #ifdef _NEXDISPLAY_DEBUG_SERIAL_
                Serial.println();
                Serial.print("Nextion Touch: 0x");
                Serial.print(__buffer[0], HEX);
                Serial.print(", 0x");
                Serial.print(__buffer[1], HEX);
                Serial.print(", 0x");
                Serial.print(__buffer[2], HEX);
                Serial.print(", 0x");
                Serial.print(__buffer[3], HEX);
                Serial.print(", 0x");
                Serial.print(__buffer[4], HEX);
                Serial.print(", 0x");
                Serial.print(__buffer[5], HEX);
                Serial.print(", 0x");
                Serial.print(__buffer[6], HEX);
                Serial.println();
              #endif
              
              //__buffer[1], __buffer[2] - pageId, elementId
              
              //process hardcoded values from 'printh' in nextion events
              if (__buffer[1] == 0x99) { 
                switch (__buffer[2]) {
                  case 0x01: onSaveClick(); break;
                  case 0x02: onChangeModeClick(); break;
                  case 0x03: onReset(); break;
                }
              }
          }
        }
      }
    }
    __criticalSection = false;
}

void NexDisplay::loadConfig() {

    //init variables from EEPROM config
    __nxSendNumber(pServ, F("nHID"), cfg.getHID());
    __nxSendNumber(pAlarm, F("vAlarmMax"), cfg.getMaxTemp());
    __nxSendNumber(pAlarm, F("vAlarmDrop"), cfg.getMaxDropTemp());
    __nxSendNumber(pTemps, F("vReqTemp"), cfg.getRequiredTemp());
    __nxSendNumber(pTemps, F("vHisteresis"), cfg.getHysteresisTemp());
    __nxSendNumber(pExh, F("vExhDiff"), cfg.getExhaustDeltaTemp());
    __nxSendNumber(pFeed, F("vFeedIgnWork"), cfg.getFeedIgnitionWorkS());
    __nxSendNumber(pFeed, F("vFeedIgnDelay"), cfg.getFeedIgnitionDelayS());
    __nxSendNumber(pFeed, F("vFeedIgnPow"), cfg.getFeedIgnitionP());
    __nxSendNumber(pFeed, F("vFeedHeatWork"), cfg.getFeedHeatWorkS());
    __nxSendNumber(pFeed, F("vFeedHeatDelay"), cfg.getFeedHeatDelayS());
    __nxSendNumber(pFeed, F("vFeedHeatPow"), cfg.getFeedHeatP());
    __nxSendNumber(pFeed, F("vFeedIdleWork"), cfg.getFeedIdleWorkS());
    __nxSendNumber(pFeed, F("vFeedIdleDelay"), cfg.getFeedIdleDelayS());
    __nxSendNumber(pFeed, F("vFeedIdlePow"), cfg.getFeedIdleP());
    __nxSendNumber(pFeed, F("vFAmpRev"), cfg.getFeedAmpsRev()); 
    __nxSendNumber(pFeed, F("vFAmpAlm"), cfg.getFeedAmpsMax()); 
    __nxSendNumber(pFan, F("vFanIgnWork"), cfg.getFanIgnitionWorkS());
    __nxSendNumber(pFan, F("vFanIgnPowOn"), cfg.getFanIgnitionOnP());
    __nxSendNumber(pFan, F("vFanIgnPowOff"), cfg.getFanIgnitionOffP());
    __nxSendNumber(pFan, F("vFanHeatPow"), cfg.getFanHeatP());
    __nxSendNumber(pFan, F("vFanIdleWork"), cfg.getFanIdleWorkS());
    __nxSendNumber(pFan, F("vFanIdlePowOn"), cfg.getFanIdleOnP());
    __nxSendNumber(pFan, F("vFanIdlePowOff"), cfg.getFanIdleOffP());
    __nxSendNumber(pFan, F("vFanClnWorkS"), cfg.getFanCleanWorkS());
    __nxSendNumber(pFan, F("vFanClnPow"), cfg.getFanCleanP());
    __nxSendNumber(pIgn, F("vFlameLevel"), cfg.getFlameLevel());
    __nxSendNumber(pIgn, F("vFlmOutS"), cfg.getFlameTimoutS());
    __nxSendNumber(pIgn, F("vIgnStart"), cfg.getIgniterStartS());
    __nxSendNumber(pIgn, F("vIgnWork"), cfg.getIgniterWorkS());
    __nxSendNumber(pIgn, F("vIgnDelay"), cfg.getIgniterDelayS());
    __nxSendNumber(pFuel, F("vFuelLvl100"), cfg.getFuelLevel(P100));
    __nxSendNumber(pFuel, F("vFuelLvl80"), cfg.getFuelLevel(P80));
    __nxSendNumber(pFuel, F("vFuelLvl60"), cfg.getFuelLevel(P60));
    __nxSendNumber(pFuel, F("vFuelLvl40"), cfg.getFuelLevel(P40));
    __nxSendNumber(pFuel, F("vFuelLvl20"), cfg.getFuelLevel(P20));
    __nxSendNumber(pFuel, F("vFuelLvl0"), cfg.getFuelLevel(P0));
    __nxSendNumber(pBat, F("vBatLvl100"), cfg.getBattLevel(P100));
    __nxSendNumber(pBat, F("vBatLvl80"), cfg.getBattLevel(P80));
    __nxSendNumber(pBat, F("vBatLvl60"), cfg.getBattLevel(P60));
    __nxSendNumber(pBat, F("vBatLvl40"), cfg.getBattLevel(P40));
    __nxSendNumber(pBat, F("vBatLvl20"), cfg.getBattLevel(P20));
    __nxSendNumber(pBat, F("vBatLvl0"), cfg.getBattLevel(P0));
    
}

void NexDisplay::refresh() {
    
    if (__criticalSection) return; //skip refresh if reading data

    __nxSendNumber(pMain, F("nCurT"), burner.getCurrentTemp());
    __nxSendNumber(pMain, F("nExhT"), burner.getExhaustTemp());
    __nxSendNumber(pMain, F("nFdP"), burner.getFeed());
    __nxSendNumber(pMain, F("nFdT"), burner.getFeederTemp());
    __nxSendNumber(pServ, F("nFdS"), burner.getFeedTime());
    __nxSendNumber(pMain, F("nFlm"), burner.getFlame());
    __nxSendNumber(pMain, F("nNoFlm"), burner.isFlame() ? 0 : burner.getSecondsWithoutFlame()); //cfg.getFlameTimoutS() - burner.getSecondsWithoutFlame()
    __nxSendNumber(pMain, F("nFan"), burner.getFan());
    __nxSendNumber(pMain, F("nBat"), burner.getBattLevel()); 
    __nxSendNumber(pMain, F("nFuel"), burner.getFuelLevel());
    __nxSendNumber(pState, F("vMode"), burner.getCurrentMode()); 
    __nxSendNumber(pState, F("vAlmStat"), burner.getAlarmStatus()); 
    __nxSendNumber(pFeed, F("nAmps"), burner.getFeederAmps()); 
    __nxSendNumber(pMain, F("vRev"), burner.isFeedReverse());
    __nxSendNumber(pBat, F("nVts"), burner.getBattDVolts()); 
    __nxSendNumber(pFuel, F("nFlCm"), burner.getFuelCm()); 
    __nxSendNumber(pMain, F("nMinT"), burner.getMinTemp()); 
    __nxSendNumber(pMain, F("nMinT"), burner.getMinTemp()); 
    __nxSendNumber(pMain, F("vIgn"), burner.isIgnition());
    __nxSendNumber(pMain, F("vUps"), burner.isPumpUPS()); 

    //if not custom message of message timout
    if (_isMessageTimout()) {
      
      String state = "";
      switch (burner.getCurrentMode()) {
        case MODE_MANUAL: state += F("Manual"); break;
        case MODE_IGNITION: state += F("Ignition"); break;
        case MODE_HEAT: state += F("Heating"); break;
        case MODE_IDLE: state += F("Waiting"); break;
        case MODE_CLEANING: state += F("Cleaning"); break;
        case MODE_ALARM: state += F("Alarm"); break;
      }
      switch (burner.getAlarmStatus()) {
        case ALARM_OK: break;
        case ALARM_MANUAL: state += F(": Manual"); break;
        case ALARM_TEMPDROP: state += F(": Temp. drop"); break;
        case ALARM_NOFLAME: state += F(": No flame"); break;
        case ALARM_IGNITION_FAILED: state += F(": Ignition failed"); break;
        case ALARM_OVERHEAT: state += F(": Overheat"); break;
        case ALARM_OVERHEAT_FEED: state += F(": Feed temp"); break;
        case ALARM_FEED_STUCK: state += F(": Feed stuck"); break;
      }
      __nxSendString(pMain, F("state"), state.c_str());
    
    }

    //chart plotting
    for (byte i = 0;i<=2;i++) {
      __nxPrintFF();
      nexSerial.print(F("add 11,"));
      nexSerial.print(i);
      nexSerial.print(" ");
      switch (i) {
        case 0: nexSerial.print(burner.getCurrentTemp()); break;
        case 1: nexSerial.print(cfg.getRequiredTemp()); break;
        case 2: nexSerial.print(burner.getExhaustTemp()); break;
      }
      __nxPrintFF();
      nexSerial.println();
    }
}

void NexDisplay::onSaveClick()
{
    short pVer = __nxReceivePacket();
    #ifdef _NEXDISPLAY_DEBUG_SERIAL_
      Serial.print("onSaveClick:");
      Serial.println(pVer);
    #endif

    switch (pVer) {
      case 101: //alarms page
        cfg.setMaxTemp(_rv[2]);
        cfg.setMaxDropTemp(_rv[3]);
        break;
      case 102: //temps page
        cfg.setRequiredTemp(_rv[2]);
        cfg.setHysteresisTemp(_rv[3]);
        break;
      case 103: //exhaust page
        cfg.setExhaustDeltaTemp(_rv[2]);
        break;
      case 104: //feeder page
        cfg.setFeedIgnitionWorkS(_rv[2]);
        cfg.setFeedIgnitionDelayS(_rv[3]);
        cfg.setFeedIgnitionP(_rv[4]);
        cfg.setFeedHeatWorkS(_rv[5]);
        cfg.setFeedHeatDelayS(_rv[6]);
        cfg.setFeedHeatP(_rv[7]);
        cfg.setFeedIdleWorkS(_rv[8]);
        cfg.setFeedIdleDelayS(_rv[9]);
        cfg.setFeedIdleP(_rv[10]);
        cfg.setFeedAmpsRev(_rv[11]);
        cfg.setFeedAmpsMax(_rv[12]);
        break;
      case 105: //fan page
        cfg.setFanIgnitionWorkS(_rv[2]);
        cfg.setFanIgnitionOnP(_rv[3]);
        cfg.setFanIgnitionOffP(_rv[4]);
        cfg.setFanHeatP(_rv[5]);
        cfg.setFanIdleWorkS(_rv[6]);
        cfg.setFanIdleOnP(_rv[7]);
        cfg.setFanIdleOffP(_rv[8]);
        cfg.setFanCleanWorkS(_rv[9]);  
        cfg.setFanCleanP(_rv[10]);  
        break;
      case 106: //ignition page
        cfg.setFlameLevel(_rv[2]);
        cfg.setFlameTimoutS(_rv[3]);
        cfg.setIgniterStartS(_rv[4]);
        cfg.setIgniterWorkS(_rv[5]);
        cfg.setIgniterDelayS(_rv[6]);
        break;
      case 107: //fuel page
        cfg.setFuelLevel(P100, _rv[2]);
        cfg.setFuelLevel(P80, _rv[3]);
        cfg.setFuelLevel(P60, _rv[4]);
        cfg.setFuelLevel(P40, _rv[5]);
        cfg.setFuelLevel(P20, _rv[6]);
        cfg.setFuelLevel(P0, _rv[7]);
        break;
      case 108: //batt page
        cfg.setBattLevel(P100, _rv[2]);
        cfg.setBattLevel(P80, _rv[3]);
        cfg.setBattLevel(P60, _rv[4]);
        cfg.setBattLevel(P40, _rv[5]);
        cfg.setBattLevel(P20, _rv[6]);
        cfg.setBattLevel(P0, _rv[7]);
        break;
    }
    
    if (pVer > 0 && cfg.store()) {
      nexdisp.loadConfig();
      burner.beep();
      _sendMessage(String(F("Saved!")).c_str());
    }
    else {
      _sendMessage(String(F("Failed!")).c_str());
    }
    
}

void NexDisplay::onChangeModeClick()
{
    __nxRecvRetNumber(&_rv[0]);
    #ifdef _NEXDISPLAY_DEBUG_SERIAL_
      Serial.print("onChangeModeClick:");
      Serial.println(_rv[0]);
    #endif
    
    switch (_rv[0]) {
      case 1: burner.setCurrentMode(MODE_MANUAL, ALARM_OK); burner.beep(); break;
      case 2: burner.setCurrentMode(MODE_IGNITION, ALARM_OK); burner.beep(); break;
      case 3: burner.setCurrentMode(MODE_HEAT, ALARM_OK); burner.beep(); break;
      case 4: burner.setCurrentMode(MODE_IDLE, ALARM_OK); burner.beep(); break;
      case 5: burner.setCurrentMode(MODE_CLEANING, ALARM_OK); burner.beep(); break;
      case 6: burner.setCurrentMode(MODE_ALARM, ALARM_MANUAL); burner.beep(); break;
      case 11: burner.setFeed(cfg.getFeedHeatP()); burner.beep(); break;
      case 10: burner.setFeed(LOW); burner.beep(); break;
      case 21: burner.setFan(cfg.getFanHeatP()); burner.beep(); break;
      case 20: burner.setFan(LOW); burner.beep(); break;
      case 30: burner.setPumpUPS(false); burner.beep(); break;
      case 31: burner.setPumpUPS(true); burner.beep(); break;
      case 40: burner.setIgnition(false); burner.beep(); break;
      case 41: burner.setIgnition(true); burner.beep(); break;
      case 50: burner.onAlarmOff(); burner.beep(); break;
      case 51: burner.onAlarmOn(); burner.beep(); break;
      case 60: burner.setFeedReverse(false); burner.beep(); break;
      case 61: burner.setFeedReverse(true); burner.beep(); break;
    }
        
}

void NexDisplay::onReset()
{
    __nxRecvRetNumber(&_rv[0]);
    #ifdef _NEXDISPLAY_DEBUG_SERIAL_
      Serial.print("onChangeModeClick:");
      Serial.println(_rv[0]);
    #endif

    if (_rv[0] == 999) { //command confirmation
      cfg.reset();
      burner.beep();
      loadConfig();
      _sendMessage(String(F("Reset!")).c_str());
      __nxSendCommand(String(F("page 0")).c_str());
    }
}

bool NexDisplay::_isMessageTimout() 
{
  if (_uiMessageTime == 0) {
    return true;
  }
  else {
    if (millis() / 1000 - _uiMessageTime > 5) {
      _uiMessageTime = 0;
      return true;
    }
    else return false;
  }
}

void NexDisplay::_sendMessage(const char* message)
{
  _uiMessageTime = millis() / 1000; 
  
  __nxSendString(pMain, F("state"), message);
}

void NexDisplay::__nxPrintFF() {
  for (byte i=1; i<=3;i++) {
    nexSerial.write(0xFF); 
  }
}

void NexDisplay::__serialPrintPage(enum NX_PAGE page) {
  switch (page) {
    case pMain: nexSerial.print(F("pMain")); break;
    case pAlarm: nexSerial.print(F("pAlarm")); break;
    case pTemps: nexSerial.print(F("pTemps")); break;
    case pExh: nexSerial.print(F("pExh")); break;
    case pFeed: nexSerial.print(F("pFeed")); break;
    case pFan: nexSerial.print(F("pFan")); break;
    case pIgn: nexSerial.print(F("pIgn")); break;
    case pFuel: nexSerial.print(F("pFuel")); break;
    case pBat: nexSerial.print(F("pBat")); break;
    case pServ: nexSerial.print(F("pServ")); break;
    case pState: nexSerial.print(F("pState")); break;
  }
}

void NexDisplay::__nxSendString(enum NX_PAGE page, const __FlashStringHelper* key, const char* value) {
  __nxPrintFF();
  __serialPrintPage(page);
  nexSerial.print(".");
  nexSerial.print(key);
  nexSerial.print(".txt=\"");
  nexSerial.print(value);
  nexSerial.print("\"");
  __nxPrintFF();
  nexSerial.println("");
}

void NexDisplay::__nxSendNumber(enum NX_PAGE page, const __FlashStringHelper* key, int value) {
  __nxPrintFF();
  __serialPrintPage(page);
  nexSerial.print(".");
  nexSerial.print(key);
  nexSerial.print(".val=");
  nexSerial.print(value);
  __nxPrintFF();
  nexSerial.println("");  
}

void NexDisplay::__nxSendCommand(const char* cmd)
{
  __nxPrintFF(); //added for USB and Nextion streams separation
  nexSerial.print(cmd);
  __nxPrintFF();
  nexSerial.println(); //added for USB and Nextion streams separation
}

bool NexDisplay::__nxRecvRetNumber(uint32_t *number)
{
    bool ret = false;
    uint8_t temp[8] = {0};

    if (!number)
    {
        goto __return;
    }
    
    nexSerial.setTimeout(TIMEOUT);
    if (sizeof(temp) != nexSerial.readBytes((char *)temp, sizeof(temp)))
    {
        goto __return;
    }

    if (temp[0] == NEX_RET_NUMBER_HEAD
        && temp[5] == 0xFF
        && temp[6] == 0xFF
        && temp[7] == 0xFF
        )
    {
        *number = ((uint32_t)temp[4] << 24) | ((uint32_t)temp[3] << 16) | (temp[2] << 8) | (temp[1]);
        ret = true;
    }

__return:
    return ret;
}

/*
 * Reads packet from Nextion: 1-packed version, 2- variables count, 3..n- variables, n+1 - checksum
 */
short NexDisplay::__nxReceivePacket()
{
    _rv[0] = 0; __nxRecvRetNumber(&_rv[0]); //packet version
    //Serial.print("0:"); Serial.println(_rv[0]);
    _rv[1] = 0; __nxRecvRetNumber(&_rv[1]); //variables count in packet
    //Serial.print("1:"); Serial.println(_rv[1]);
    
    if (_rv[1] < 0 || _rv[1] > sizeof(_rv) - 3) return -1; // check for out of range, -3 is version, size, checksum. 
    if (_rv[1] == 0) return (short)_rv[0]; //check for variables //TODO: should not be comparation to 0, 0 on read error. need offset, e.g. +100
    
    uint32_t iCheckSum = 11; //checksum offset in case of checksum read failed. 
    for (byte i=2; i < _rv[1] + 3; i++) { 
      _rv[i] = 0;
      if (!__nxRecvRetNumber(&_rv[i])) return -2;
      //Serial.print(":"); Serial.println(_rv[i]);
      if (i-2 != _rv[1]) iCheckSum += _rv[i]; //skip check sum
    }

    #ifdef _NEXDISPLAY_DEBUG_SERIAL_
      for (byte x=0; x < 15; x++) { 
        Serial.print(x); 
        Serial.print(":"); 
        Serial.println(_rv[x]);
      }
      Serial.println(_rv[_rv[1]+2]);
      Serial.println(iCheckSum);
    #endif
    
    if (_rv[_rv[1]+2] != iCheckSum) return -3; 
    
    return (short)_rv[0]; 
}


/*
    To separate logging stream from Nextion communication same TX/RX pins I've added to NexHardware.cpp - sendCommand:
    nexSerial.write(0xFF); //added 
    nexSerial.write(0xFF); //added 
    nexSerial.write(0xFF); //added 
    nexSerial.print(cmd);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
    nexSerial.println();   //added 
 */

