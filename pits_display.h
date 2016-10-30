#ifndef _PITSDISPLAY_H_
#define _PITSDISPLAY_H_

#include "RTClib.h"
#include <ACROBOTIC_SSD1306.h>

#include "pits_burner.h"
extern PitsBurner burner;

#include <TaskScheduler.h>
extern Scheduler scheduler;

extern RTC_DS3231 rtc;

class PitsDisplay;
extern PitsDisplay display;

class PitsDisplay
{
  public:
    PitsDisplay();
    void init();
    static void refresh();
  private:
    bool _animationFrame = true;
};

#endif


