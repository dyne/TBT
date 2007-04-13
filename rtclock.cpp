/*  Time Based Text - Real Time Clock for Linux
 *
 *  (C) Copyright 2006 - 2007 Denis Rojo <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef linux

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>


#ifdef linux
/* we try to use the realtime linux clock on /dev/rtc */
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <inttypes.h>

#include <rtclock.h>

#include <jutils.h>

// threading stuff
typedef void* (kickoff)(void*);


RTClock::RTClock() {  
  rtcfd = 0;
  quit = false;
  msec = 0;
  _thread = 0x0;
  sleeping = false;
}


RTClock::~RTClock() {

  if(_thread) {
    quit = true;
    pthread_join(_thread, NULL);
  }

  // close rtc
  if(rtcfd>0) {
    ioctl(rtcfd, RTC_UIE_OFF, 0);
    //  ioctl(rtcfd,RTC_PIE_OFF,0);
    ::close(rtcfd);
  }
  
  // close posix threads
  if(pthread_mutex_destroy(&_mutex) == -1)
    error("error destroying POSIX thread mutex");
  if(pthread_cond_destroy(&_cond) == -1)
    error("error destroying POSIX thread condition");
  if(pthread_attr_destroy(&_attr) == -1)
    error("error destroying POSIX thread attribute");

}

int RTClock::init() {
  int res;

  // open RTC device
  rtcfd = ::open("/dev/rtc",O_RDONLY);
  if(rtcfd<0) {
    warning("couldn't open real time clock device: %s", strerror(errno));
    warning("reverting to POSIX.1 time handling");
    return 0;
  }
  /* set the alarm event to 1 second */
  res = ioctl(rtcfd, RTC_UIE_ON, 0);
  if(res<0) {
    error("couldn't set real time clock device tick: %s", strerror(errno));
    return 0;
  }

  // initialize thread and rtc
  if(pthread_attr_init (&_attr) == -1)
    error("error initializing POSIX thread attribute");
  if(pthread_mutex_init (&_mutex,NULL) == -1)
    error("error initializing POSIX thread mutex");
  if(pthread_cond_init (&_cond, NULL) == -1)
    error("error initializing POSIX thread condtition"); 
  
  /* sets the thread as detached
     see: man pthread_attr_init(3) */
  //  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_DETACHED);
  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_JOINABLE);


  notice("real time clock succesfully initialized");
  
  return 1;
}

int RTClock::set_freq(unsigned long freq) {
  int res;

  if(rtcfd<0) {
    error("can't set clock frequency: real time clock device is not open");
    return -1;
  }
  
  res = ioctl(rtcfd,RTC_IRQP_SET,freq);

  if(res<0)
    error("RTC_IRQP_SET failed: %s", strerror(errno));

  else {

    res = ioctl(rtcfd,RTC_IRQP_READ,&freq);

    if(res<0)
      error("RTC_IRQP_READ failed: %s", strerror(errno));

    else {
      // enable periodic interrupts
      res = ioctl(rtcfd,RTC_PIE_ON,0);
      if(res<0)
	error("RTC_PIE_ON failed: %s", strerror(errno));
      else
	act("realtime clock frequency set to %lu",freq);
    }
  }

  // reset clock
  quit = false;
  msec = 0;

  return res;
}

int RTClock::tick() {
  int res;

  // blocking read on /dev/rtc
  res = read(rtcfd, &rtctime, sizeof(unsigned long));
  if(res < 0)
    error("problem catching real time clock interrupt: %s", strerror(errno));

  // update clock counter
  msec++;
  
  return res;
}

void RTClock::run() {

  // run the clock
  while(!quit) {

    if(sleeping) wait();
    
    tick(); // blocking
    
  }

}



int RTClock::start() {
  int res;

  if(rtcfd<0) {
    error("can't start clock: real time clock device is not open");
    return -1;
  }

  if(_thread) {
    quit = true;
    pthread_join(_thread, NULL);
  }

  // reset clock
  quit = false;
  msec = 0;

  res = pthread_create(&_thread, &_attr, &kickoff, this);
  if(res != 0) {
    error("can't create thread: %s",strerror(errno));
    return 0;
  }
  return 1;
}

int RTClock::sleep(uint64_t msec) {
  register uint64_t elap_msec;

  if(rtcfd<0) {
    error("tbt::clock::sleep - real time clock device is not open");
    return -1;
  }

  // tell main thread to wait()
  sleeping = true;

  // microseconds loop
  for(elap_msec = msec; elap_msec >0; elap_msec--)
    tick(); // blocking

  sleeping = false;
  signal(); // main thread can run

  return 1;
}

#endif
