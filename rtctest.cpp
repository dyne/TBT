#ifndef linux

#error "rtctest can be compiled only on linux"

#else

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <tbt.h>
#include <jutils.h>
#include <rtclock.h>



RTClock tbtclock;

int main(int artc, char **argv) {

  int c;

  notice("Testing real time clock");

  tbtclock.init();

  if( tbtclock.set_freq( 1024L ) < 0) {
    error("can't access real time clock. have you loaded the rtc kernel module?");
    exit(0);
  }

  act("starting clock thread");

  tbtclock.start();
  

  for(c=0; c<10; c++) {
    tbtclock.sleep(500);
    act("%u check: %lu microsecs", c, tbtclock.msec);
  }

  act("sleeping two seconds while the clock runs..");
  sleep(2);

  for(c=0; c<10; c++) {
    tbtclock.sleep(1000);
    act("%u check: %lu microsecs", c, tbtclock.msec);
  }

  exit(1);
}

#endif
