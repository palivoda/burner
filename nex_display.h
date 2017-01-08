/*
  nex_display - Class to interface pits_burner with nextion display.
  Created by Rostislav Palivoda, December 5, 2016.
  GNU GPL3 Licence.
*/
#ifndef _NEXDISPLAY_H_
#define _NEXDISPLAY_H_

#include <Arduino.h>

//#define _NEXDISPLAY_DEBUG_SERIAL_


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

class NexDisplay
{

  public:
    void init();
    void loop();
    void loadConfig();
    void refresh();
    void onChangeModeClick(); //manual mode change
    void onSaveClick();       //save settings
    void onReset();           //save settings
    
  private:
    void __nxPrintFF();
    void __readSerial();
    void __serialPrintPage(enum NX_PAGE page);
    void __nxSendString(enum NX_PAGE page, const __FlashStringHelper* key, const char* value);
    void __nxSendNumber(enum NX_PAGE page, const __FlashStringHelper* key, int value);
    void __nxSendCommand(const char* cmd);
    bool __nxRecvRetNumber(uint32_t *number);
    short __nxReceivePacket();
    bool __criticalSection;

    short _uiMessageTime = 0;
    bool _isMessageTimout();
    void _sendMessage(const char*);
};

#define TIMEOUT                             500       //ms
#define NEX_RET_CMD_FINISHED                (0x01)
#define NEX_RET_EVENT_LAUNCHED              (0x88)
#define NEX_RET_EVENT_UPGRADED              (0x89)
#define NEX_RET_EVENT_TOUCH_HEAD            (0x65)     
#define NEX_RET_EVENT_POSITION_HEAD         (0x67)
#define NEX_RET_EVENT_SLEEP_POSITION_HEAD   (0x68)
#define NEX_RET_CURRENT_PAGE_ID_HEAD        (0x66)
#define NEX_RET_STRING_HEAD                 (0x70)
#define NEX_RET_NUMBER_HEAD                 (0x71)
#define NEX_RET_INVALID_CMD                 (0x00)
#define NEX_RET_INVALID_COMPONENT_ID        (0x02)
#define NEX_RET_INVALID_PAGE_ID             (0x03)
#define NEX_RET_INVALID_PICTURE_ID          (0x04)
#define NEX_RET_INVALID_FONT_ID             (0x05)
#define NEX_RET_INVALID_BAUD                (0x11)
#define NEX_RET_INVALID_VARIABLE            (0x1A)
#define NEX_RET_INVALID_OPERATION           (0x1B)

#endif
