/*
  burner_ethernet - Web interface for burner. Can be part of smart home.  
  Created by Rostislav Palivoda, December 7, 2016.
  GNU GPL3 Licence.
 */
#ifndef _BURNER_ETHERNET_H_
#define _BURNER_ETHERNET_H_

#include <Arduino.h>

class BurnerEthernet 
{
  public:
    void init();
    void listen();
    const byte* getMac();
};

#endif
