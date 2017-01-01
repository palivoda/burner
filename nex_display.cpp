#include "NexConfig.h"
#include "NexTouch.h"
#include "NexHardware.h"

#include "nex_display.h"
NexDisplay nexdisp;

#include "pits_burner.h"
extern PitsBurner burner;

#include "burner_config.h"
extern BurnerConfig cfg;

static uint32_t _rv = 0; //number read buffer
const short _timeout = 2500; //ms

void nxPrintFF() {
    for (byte i=1; i<=3;i++) {
      nexSerial.write(0xFF); 
    }
}
    
void nxSendMessage(const char* value) {
  nxPrintFF();
  nexSerial.print(F("pMain.state.txt"));
  nexSerial.print("=\"");
  nexSerial.print(value);
  nexSerial.print("\"");
  nxPrintFF();
  nexSerial.println("");
}

enum NX_PAGE{
  pMain, 
  pAlarm, 
  pTemps, 
  pExh,
  pFeed,
  pFan, 
  pIgn,
  pFuel, 
  pBat,
  pServ,
  pState
};

void nxSendValue(enum NX_PAGE page, const __FlashStringHelper* key, int value) {
  nxPrintFF();
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
  nexSerial.print(".");
  nexSerial.print(key);
  nexSerial.print(".val=");
  nexSerial.print(value);
  nxPrintFF();
  nexSerial.println("");  
}

//WARNING: if you change page orders or elements ID in nextion then event handlers will stop working
NexTouch bSaveConfig = NexTouch(1, 1, ""); //this is hardcoded pageId and elementId in click events of all save config buttons
NexTouch bBurnerMode = NexTouch(11, 2, ""); //this is hardcoded pageId and elementId on buttons click in service page

NexTouch *nex_listen_list[] = 
{
    &bSaveConfig,
    &bBurnerMode,
    NULL
};

void bSaveCallback(void *ptr)
{
    recvRetNumber(&_rv, _timeout);
    bool bRead = false;

    switch (_rv) {
      case 101: //alarms page
        recvRetNumber(&_rv, _timeout); cfg.setMaxTemp(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setMaxDropTemp(_rv);
        bRead = true;
        break;
      case 102: //temps page
        recvRetNumber(&_rv, _timeout); cfg.setRequiredTemp(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setHysteresisTemp(_rv);
        bRead = true;
        break;
      case 103: //exhaust page
        recvRetNumber(&_rv, _timeout); cfg.setExhaustDeltaTemp(_rv);
        bRead = true;
        break;
      case 104: //feeder page
        recvRetNumber(&_rv, _timeout); cfg.setFeedIgnitionWorkS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedIgnitionDelayS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedIgnitionP(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedHeatWorkS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedHeatDelayS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedHeatP(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedIdleWorkS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedIdleDelayS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedIdleP(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedAmpsRev(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFeedAmpsMax(_rv);
        bRead = true;
        break;
      case 105: //fan page
        recvRetNumber(&_rv, _timeout); cfg.setFanIgnitionWorkS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFanIgnitionOnP(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFanIgnitionOffP(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFanHeatP(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFanIdleOnP(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFanIdleWorkS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFanIdleOffP(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFanCleanWorkS(_rv);  
        recvRetNumber(&_rv, _timeout); cfg.setFanCleanP(_rv);  
        bRead = true;
        break;
      case 106: //ignition page
        recvRetNumber(&_rv, _timeout); cfg.setFlameLevel(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setFlameTimoutS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setIgniterStartS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setIgniterWorkS(_rv);
        recvRetNumber(&_rv, _timeout); cfg.setIgniterDelayS(_rv);
        bRead = true;
        break;
      case 107: //fuel page
        recvRetNumber(&_rv, _timeout); cfg.setFuelLevel(P100, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setFuelLevel(P80, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setFuelLevel(P60, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setFuelLevel(P40, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setFuelLevel(P20, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setFuelLevel(P0, _rv);
        bRead = true;
        break;
      case 108: //batt page
        recvRetNumber(&_rv, _timeout); cfg.setBattLevel(P100, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setBattLevel(P80, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setBattLevel(P60, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setBattLevel(P40, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setBattLevel(P20, _rv);
        recvRetNumber(&_rv, _timeout); cfg.setBattLevel(P0, _rv);
        bRead = true;
        break;
      case 999: //service page - reset
        cfg.reset();
        bRead = true;
        break;
    }
    
    String msg;
    if (bRead && cfg.store()) {
      msg="Saved!";
      nexdisp.loadConfig();
      burner.beep();
    }
    else msg="Failed!";
    
    nxSendMessage(msg.c_str());
}

void bBurnerModeCallback(void *ptr)
{
    recvRetNumber(&_rv, _timeout);
    switch (_rv) {
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

void NexDisplay::init() {
  
    nexInit();
    
    bSaveConfig.attachPop(bSaveCallback);
    bBurnerMode.attachPop(bBurnerModeCallback);
    
    loadConfig();
    
    /*
    nxSendValue(F("rtc0"), ); //year
    nxSendValue(F("rtc1"), ); //month
    nxSendValue(F("rtc2"), ); //date
    nxSendValue(F("rtc3"), ); //hour
    nxSendValue(F("rtc4"), ); //minute
    */
    
    refresh();
}

void NexDisplay::loadConfig() {
    
    //init variables from EEPROM config
    nxSendValue(pAlarm, F("vAlarmMax"), cfg.getMaxTemp());
    nxSendValue(pAlarm, F("vAlarmDrop"), cfg.getMaxDropTemp());
    nxSendValue(pTemps, F("vReqTemp"), cfg.getRequiredTemp());
    nxSendValue(pTemps, F("vHisteresis"), cfg.getHysteresisTemp());
    nxSendValue(pExh, F("vExhDiff"), cfg.getExhaustDeltaTemp());
    nxSendValue(pFeed, F("vFeedIgnWork"), cfg.getFeedIgnitionWorkS());
    nxSendValue(pFeed, F("vFeedIgnDelay"), cfg.getFeedIgnitionDelayS());
    nxSendValue(pFeed, F("vFeedIgnPow"), cfg.getFeedIgnitionP());
    nxSendValue(pFeed, F("vFeedHeatWork"), cfg.getFeedHeatWorkS());
    nxSendValue(pFeed, F("vFeedHeatDelay"), cfg.getFeedHeatDelayS());
    nxSendValue(pFeed, F("vFeedHeatPow"), cfg.getFeedHeatP());
    nxSendValue(pFeed, F("vFeedIdleWork"), cfg.getFeedIdleWorkS());
    nxSendValue(pFeed, F("vFeedIdleDelay"), cfg.getFeedIdleDelayS());
    nxSendValue(pFeed, F("vFeedIdlePow"), cfg.getFeedIdleP());
    nxSendValue(pFan, F("vFanIgnWork"), cfg.getFanIgnitionWorkS());
    nxSendValue(pFan, F("vFanIgnPowOn"), cfg.getFanIgnitionOnP());
    nxSendValue(pFan, F("vFanIgnPowOff"), cfg.getFanIgnitionOffP());
    nxSendValue(pFan, F("vFanHeatPow"), cfg.getFanHeatP());
    nxSendValue(pFan, F("vFanIdleWork"), cfg.getFanIdleWorkS());
    nxSendValue(pFan, F("vFanIdlePowOn"), cfg.getFanIdleOnP());
    nxSendValue(pFan, F("vFanIdlePowOff"), cfg.getFanIdleOffP());
    nxSendValue(pFan, F("vFanClnWorkS"), cfg.getFanCleanWorkS());
    nxSendValue(pFan, F("vFanClnPow"), cfg.getFanCleanP());
    nxSendValue(pIgn, F("vFlameLevel"), cfg.getFlameLevel());
    nxSendValue(pIgn, F("vFlmOutS"), cfg.getFlameTimoutS());
    nxSendValue(pIgn, F("vIgnStart"), cfg.getIgniterStartS());
    nxSendValue(pIgn, F("vIgnWork"), cfg.getIgniterWorkS());
    nxSendValue(pIgn, F("vIgnDelay"), cfg.getIgniterDelayS());
    nxSendValue(pFuel, F("vFuelLvl100"), cfg.getFuelLevel(P100));
    nxSendValue(pFuel, F("vFuelLvl80"), cfg.getFuelLevel(P80));
    nxSendValue(pFuel, F("vFuelLvl60"), cfg.getFuelLevel(P60));
    nxSendValue(pFuel, F("vFuelLvl40"), cfg.getFuelLevel(P40));
    nxSendValue(pFuel, F("vFuelLvl20"), cfg.getFuelLevel(P20));
    nxSendValue(pFuel, F("vFuelLvl0"), cfg.getFuelLevel(P0));
    nxSendValue(pBat, F("vBatLvl100"), cfg.getBattLevel(P100));
    nxSendValue(pBat, F("vBatLvl80"), cfg.getBattLevel(P80));
    nxSendValue(pBat, F("vBatLvl60"), cfg.getBattLevel(P60));
    nxSendValue(pBat, F("vBatLvl40"), cfg.getBattLevel(P40));
    nxSendValue(pBat, F("vBatLvl20"), cfg.getBattLevel(P20));
    nxSendValue(pBat, F("vBatLvl0"), cfg.getBattLevel(P0));
    
}

void NexDisplay::refresh() {

    nxSendValue(pMain, F("nCurT"), burner.getCurrentTemp());
    nxSendValue(pMain, F("nExhT"), burner.getExhaustTemp());
    nxSendValue(pMain, F("nFdP"), burner.getFeed());
    nxSendValue(pMain, F("nFdT"), burner.getFeederTemp());
    nxSendValue(pServ, F("nFdS"), burner.getFeedTime());
    nxSendValue(pMain, F("nFlm"), burner.getFlame());
    nxSendValue(pMain, F("nNoFlm"), burner.isFlame() ? 0 : burner.getSecondsWithoutFlame()); //cfg.getFlameTimoutS() - burner.getSecondsWithoutFlame()
    nxSendValue(pMain, F("nFan"), burner.getFan());
    nxSendValue(pMain, F("nBat"), burner.getBattLevel()); 
    nxSendValue(pMain, F("nFuel"), burner.getFuelLevel());
    nxSendValue(pState, F("vMode"), burner.getCurrentMode()); 
    nxSendValue(pState, F("vAlmStat"), burner.getAlarmStatus()); 
    //TODO: pump indication nxSendValue(F("pMain.???"), burner.isPump());
    nxSendValue(pFeed, F("nAmps"), burner.getFeederAmps()*10); 
    nxSendValue(pMain, F("vRev"), burner.isFeedReverse());
    nxSendValue(pBat, F("nVts"), burner.getBattDVolts()); 
    nxSendValue(pFuel, F("nFlCm"), burner.getFuelCm()); 
    nxSendValue(pMain, F("nMinT"), burner.getMinTemp()); 
    

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
      case ALARM_TEMPDROP: state += F(": Temp. drop"); break;
      case ALARM_NOFLAME: state += F(": No flame"); break;
      case ALARM_IGNITION_FAILED: state += F(": Ignition failed"); break;
      case ALARM_OVERHEAT: state += F(": Overheat"); break;
      case ALARM_OVERHEAT_FEED: state += F(": Feed temp"); break;
      case ALARM_MANUAL: state += F(": Manual"); break;
    }
    nxSendMessage(state.c_str());

    //WARNING: to fast charts refresh will broke chart, ok on 8 secs interaval
    for (byte i = 0;i<=2;i++) {
      nxPrintFF();
      nexSerial.print(F("add 11,"));
      nexSerial.print(i);
      nexSerial.print(" ");
      switch (i) {
        case 0: nexSerial.print(burner.getCurrentTemp()); break;
        case 1: nexSerial.print(cfg.getRequiredTemp()); break;
        case 2: nexSerial.print(burner.getExhaustTemp()); break;
      }
      nxPrintFF();
      nexSerial.println();
    }
}


void NexDisplay::loop() {
  nexLoop(nex_listen_list);
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

