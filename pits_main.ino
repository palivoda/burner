
#include "nex_display.h"
extern NexDisplay nexdisp;

#include "pits_burner.h"
extern PitsBurner burner;

#include "burner_config.h"
extern BurnerConfig cfg;

#include "burner_ethernet.h"
extern BurnerEthernet net;

#include <TaskScheduler.h>
Scheduler scheduler;

void onPulse();
Task tRefresh(1*TASK_SECOND, TASK_FOREVER, &onPulse, &scheduler, true);

void setup() {
  Serial.begin(9600); //same as Nextion baud rate (its on same RX/TX pins
  //cfg.reset(); //uncommend once if you need to reset config
  cfg.load();
  nexdisp.init();
  burner.init(); 
  //net.init();
}

void loop() {
  nexdisp.loop(); //RX/TX pins are shared between Nextion display and USB! 
  scheduler.execute();
  //net.listen();
}

void onPulse() {
  burner.operate();
   nexdisp.refresh();
}

