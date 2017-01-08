#include "burner_ethernet.h"
BurnerEthernet net;

#include "burner_config.h"
extern BurnerConfig cfg;

#include "pits_burner.h"
extern PitsBurner burner;

#include <EtherCard.h>
const char website[] PROGMEM = "logs-01.loggly.com";
const byte ecoPelletMac[] = { 0x1E, 0x98, 0xB5, 0xDF, 0x9A, 0x43 }; //not IEEE mac, just random
byte Ethernet::buffer[400]; //was 500, 400 is minimum for DHCP
BufferFiller bfill;

void BurnerEthernet::init() 
{ 
  #ifdef _BURNER_ETHERNET_DEBUG_SERIAL_
    Serial.println(F("Net init"));
  #endif
  __isReady = true;
  
  if (ether.begin(sizeof Ethernet::buffer, ecoPelletMac) == 0) {
    #ifdef _BURNER_ETHERNET_DEBUG_SERIAL_
      Serial.println(F("Ethernet setup failed"));
    #endif
    __isReady = false;
  }
    
  if (!ether.dhcpSetup("EcoPellet", true)) {
    #ifdef _BURNER_ETHERNET_DEBUG_SERIAL_
      Serial.println(F("DHCP failed"));
    #endif
    
    const byte ecoPelletIp[] = {192, 168, 3, 66}; 
    const byte ecoPelletGW[] = {192, 168, 3, 1}; 
    const byte ecoPelletDNS[] = {8, 8, 8, 8}; 
    if(!ether.staticSetup(ecoPelletIp, ecoPelletGW, ecoPelletDNS)) {
      __isReady = false;
    }
  }
  
  //#ifdef _BURNER_ETHERNET_DEBUG_SERIAL_
    //ether.printIp("MAC: ", ecoPelletMac);
    ether.printIp("IP: ", ether.myip);
    ether.printIp("Mask: ", ether.netmask);
    ether.printIp("GW: ", ether.gwip);
    ether.printIp("DNS: ", ether.dnsip);
  //#endif
  
  // use DNS to resolve the website's IP address
  if (!ether.dnsLookup(website)) {
    #ifdef _BURNER_ETHERNET_DEBUG_SERIAL_
      Serial.println(F("DNS failed"));
    #endif
    __isReady = false;  
  }

  if (__isReady == false) {
    digitalWrite(LED_BUILTIN, HIGH); //its D13 pin that should be used by Ethernet module only!
  }
}

static word writeResponse() {

  long t = millis() / 1000;
  word h = t / 3600;
  byte m = (t / 60) % 60;
  byte s = t % 60;

  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<meta http-equiv='refresh' content='1'/>"
    "{MdS:$D"
    ",CurT:$D"
    ",ExhT:$D"
    ",FedT:$D"
    ",Flame:$D"
    ",Fan:$D"
    ",Feed:$D"
    ",FeedA:$D"
    ",FeedT:$D"
    ",Ign:$D"
    ",NoFlm:$D"
    ",Alarm:$D"
    ",Mode:$D"
    ",Fuel:$D"
    ",Batt:$D"
    ",MaxT:$D"
    ",MaxDT:$D"
    ",ReqT:$D"
    ",HstT:$D"
    ",ExhDT:$D"
    ",UpTm:$D$D:$D$D:$D$D"
    ",HID:$D"
    "}"),
    burner.getSecondsInCurrentMode(),
    burner.getCurrentTemp(),
    burner.getExhaustTemp(),
    burner.getFeederTemp(),
    burner.getFlame(),
    burner.getFan(),
    burner.getFeed(),
    burner.getFeederAmps(),
    burner.getFeedTime(),
    burner.isIgnition(),
    burner.getSecondsWithoutFlame(),
    burner.getAlarmStatus(),
    burner.getCurrentMode(),
    burner.getFuelLevel(),
    burner.getBattLevel(),
    cfg.getMaxTemp(),
    cfg.getMaxDropTemp(),
    cfg.getRequiredTemp(),
    cfg.getHysteresisTemp(),
    cfg.getExhaustDeltaTemp(),
    h/10, h%10, m/10, m%10, s/10, s%10,
    cfg.getHID()
    );

  return bfill.position();
}

// called when the client request is complete
// called when the client request is complete
static void httpGetCallback(byte status, word off, word len) {
  #ifdef _BURNER_ETHERNET_DEBUG_SERIAL_NETWORK
    Serial.println(">>>");
    Ethernet::buffer[off+300] = 0;
    Serial.print((const char*) Ethernet::buffer + off);
    Serial.println("...");
  #endif
}

void BurnerEthernet::logme(String msg) {

  //if (!isReady()) return;
   
  char getMsg[msg.length() + 30]; 
  int ms = (int) roundf(millis() / 1000.0);
  sprintf(getMsg, "?s=%d&HID=%d&msg=", ms, cfg.getHID());
  strcat(getMsg, msg.c_str());

  //escape some URL chars
  int j = 0;
  while (getMsg[j] != 0){
      if (getMsg[j] == ' ') getMsg[j] = '+';
      if (getMsg[j] == '>') getMsg[j] = 'G';
      if (getMsg[j] == '<') getMsg[j] = 'L';
      j++;
  }
  
  #ifdef _BURNER_ETHERNET_DEBUG_SERIAL_NETWORK
    Serial.print(F("sendlog:"));
    Serial.println(getMsg);
  #endif
  
  ether.browseUrl(PSTR("/inputs/94923e52-d5d0-49d3-8f4d-22afc69efdc7.gif"), getMsg, website, httpGetCallback);
}

void BurnerEthernet::listen() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if (pos)  { // check if valid tcp data is received
    #ifdef _BURNER_ETHERNET_DEBUG_SERIAL_NETWORK
      Serial.print(F("writeResponse"));
    #endif
    ether.httpServerReply(writeResponse()); // send web page data
  }
}

const byte* BurnerEthernet::getMac() {
  return ecoPelletMac;
}

