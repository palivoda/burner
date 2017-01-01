/*
  nex_display - Class to interface pits_burner with nextion display.
  Created by Rostislav Palivoda, December 5, 2016.
  GNU GPL3 Licence.
*/
#ifndef _NEXDISPLAY_H_
#define _NEXDISPLAY_H_

#include <Arduino.h>

class NexDisplay
{
  public:
    void init();
    void loadConfig();
    void refresh();
    void loop();
};

#endif
