#include "burner_ethernet.h"
BurnerEthernet net;

#include "burner_config.h"
extern BurnerConfig cfg;

#include "pits_burner.h"
extern PitsBurner burner;

#include <EtherCard.h>
const byte ecoPelletMac[] = {0x1E, 0x98, 0xB5, 0xDF, 0x9A, 0x42}; //not IEEE mac, just random
//const byte ecoPelletIp[] = {192, 168, 3, 66}; 
byte Ethernet::buffer[300]; //was 500
BufferFiller bfill;

void BurnerEthernet::init() 
{ 
  Serial.println(F("Net init"));
  
  if (ether.begin(sizeof Ethernet::buffer, ecoPelletMac) == 0) {
    Serial.println(F("Ethernet setup failed"));
    digitalWrite(LED_BUILTIN, HIGH);
  }
    
  if (!ether.dhcpSetup("EcoPellet", true)) {
    Serial.println(F("DHCP failed"));
    digitalWrite(LED_BUILTIN, HIGH);
  }
  //ether.staticSetup(ecoPelletIp);
  
  //ether.printIp("MAC: ", ecoPelletMac);
  ether.printIp("IP: ", ether.myip);
  ether.printIp("Mask: ", ether.netmask);
  ether.printIp("GW: ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);
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
    "{CurT:$D"
    ",ExhT:$D"
    ",FedT:$D"
    ",Flame:$D"
    ",Fan:$D"
    ",Feed:$D"
    ",FeedS:$D"
    ",Ign:$D"
    ",NoFlm:$D"
    ",Alrm:$D"
    ",Mode:$D"
    ",Fuel:$D"
    ",Batt:$D"
    ",MaxT:$D"
    ",MxDT:$D"
    ",ReqT:$D"
    ",HstT:$D"
    ",ExdT:$D"
    ",UpTm:$D$D:$D$D:$D$D"
    "}"),
    burner.getCurrentTemp(),
    burner.getExhaustTemp(),
    burner.getFeederTemp(),
    burner.getFlame(),
    burner.getFan(),
    burner.getFeed(),
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
    h/10, h%10, m/10, m%10, s/10, s%10
    );

  return bfill.position();
}

void BurnerEthernet::listen() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if (pos)  // check if valid tcp data is received
    ether.httpServerReply(writeResponse()); // send web page data
}

const byte* BurnerEthernet::getMac() {
  return ecoPelletMac;
}

