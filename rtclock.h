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

#ifndef __RTCLOCK_H__
#define __RTCLOCK_H__


#include <pthread.h>


/*
  Clock timer abstraction
  using RTC in Linux/BSD
  multithreaded
 */
class RTClock {
 public:
  RTClock();
  ~RTClock();

  int init();

  int set_freq(unsigned long freq);

  int start();

  void run();

  int sleep(uint64_t sec);

  bool quit;

  uint64_t msec;

 private:

  int tick();

  int rtcfd;
  unsigned long rtctime;

  pthread_t _thread;
  pthread_attr_t _attr;

  pthread_mutex_t _mutex;
  pthread_cond_t _cond;

  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };

  /* MUTEX MUST BE LOCKED AND UNLOCKED WHILE USING WAIT */
  void wait() { pthread_cond_wait(&_cond,&_mutex); };
  void signal() { pthread_cond_signal(&_cond); };

  bool sleeping;

 protected:
  // threading stuff
  static void* kickoff(void *arg) {
    ((RTClock *) arg)->run();
    return NULL;
  };
  
};



#endif
