
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <tbt.h>
#include <jutils.h>



TBTClock tbtclock;

int main(int artc, char **argv) {


  int c;

  notice("Instantiating clock");

  tbtclock.init();

  if( tbtclock.set_freq( 1024 ) < 0) {
    error("can't access real time clock. have you loaded the rtc kernel module?");
    exit(0);
  }

  act("starting clock thread");

  tbtclock.start();
  //tbtclock->run();


  for(c=0; c<10; c++) {
    sleep(1);
    //    act("%u check: %lu secs %lu microsecs", c, tbtclock.sec, tbtclock.microsec);
  }

  exit(1);
}
