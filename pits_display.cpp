#include "pits_display.h"

PitsDisplay display;

Task tRefresh(TASK_SECOND/4, TASK_FOREVER, &PitsDisplay::refresh, &scheduler, true);

static const unsigned char ICON_BLANK[] PROGMEM ={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static const unsigned char ICON_FAN1[] PROGMEM ={
0x00,0x20,0x26,0x18,0x18,0x64,0x04,0x00
};
static const unsigned char ICON_FAN2[] PROGMEM ={
0x00,0x08,0x08,0x78,0x1E,0x10,0x10,0x00
};
static const unsigned char ICON_FIRE1[] PROGMEM ={
0x00,0xC0,0xE8,0xD0,0xE8,0xD0,0xC0,0x00
};
static const unsigned char ICON_FIRE2[] PROGMEM ={
0x00,0xC0,0xD0,0xE8,0xD0,0xE8,0xC0,0x00
};
static const unsigned char ICON_FEEDER1[] PROGMEM ={
0x00,0x10,0x18,0x30,0x18,0x30,0x18,0x00
};
static const unsigned char ICON_FEEDER2[] PROGMEM ={
0x00,0x18,0x30,0x18,0x30,0x18,0x30,0x00
};

PitsDisplay::PitsDisplay() {
  
}

void PitsDisplay::init() {
  Serial.print("PitsDisplay:");

  //init display
  oled.init();                      // Initialze SSD1306 OLED display
  oled.clearDisplay();              // Clear screen

  Serial.println(" Initiated.");
}

void PitsDisplay::refresh() {
  
  display._animationFrame = !display._animationFrame;

  //Required Temperature
  oled.setTextXY(0,0);
  oled.putString("-->");
  oled.setTextXY(1,0);
  oled.putNumber(burner.getRequiredTemp());
  
  //Current Temperature
  oled.setTextXY(0,4);              
  oled.putString("[=]");
  oled.setTextXY(1,4);
  oled.putNumber(burner.getCurrentTemp());

  //Feeder Temperature
  oled.setTextXY(0,9);              
  oled.drawBitmap((unsigned char*) (ICON_FEEDER1) ,8);
  oled.setTextXY(1,8);
  oled.putNumber(burner.getFeederTemp());
  
  //Exhaust temperature
  oled.setTextXY(0,13);
  oled.putString("| |");
  oled.setTextXY(1,13);
  oled.putNumber(burner.getExhaustTemp());
  oled.setTextXY(3,0);

  //Sensor Value
  oled.setTextXY(2,8);
  oled.putNumber(burner.getFlame());
  oled.setTextXY(2,burner.getFlame() > 9 ? 10 : 9);
  oled.putString("% ");

  //Mode
  oled.setTextXY(7,4); 
  switch (burner.getCurrentMode()) {
    case MODE_MANUAL: oled.putString("M"); break;
    case MODE_IGNITION: oled.putString("I"); break;
    case MODE_HEAT: oled.putString("H"); break;
    case MODE_IDLE: oled.putString("W"); break;
    case MODE_CLEANING: oled.putString("C"); break;
    case MODE_ALARM: oled.putString("A"); break;
  }

  //Flame
  oled.setTextXY(7,0);
  if (burner.isFlame()) {
    oled.drawBitmap((unsigned char*) (display._animationFrame ? ICON_FIRE1 : ICON_FIRE2) ,8);
  }
  else {
    oled.putString(burner.getFlame() == 0 ? "!" : "~");  
  }

  //Fan
  oled.setTextXY(7,1); 
  if (burner.getFan() > 1) oled.drawBitmap((unsigned char*) (display._animationFrame ? ICON_FAN1 : ICON_FAN2) ,8);
  else oled.putString(" ");

  //Feeder
  oled.setTextXY(7,2); 
  if (burner.getFeed()) oled.drawBitmap((unsigned char*) (display._animationFrame ? ICON_FEEDER1 : ICON_FEEDER2) ,8);
  else oled.putString(" ");
  

  //Real time
  DateTime now = rtc.now();
  String timeFormated = String(now.hour())+":"+String(now.minute())+":"+String(now.second());
  while (timeFormated.length() < 8) timeFormated = " " + timeFormated;
  char char_array[9];
  timeFormated.toCharArray(char_array, sizeof(char_array));
  oled.setTextXY(7,8); 
  oled.putString(char_array);
}



