/*  Time Based Text - Recorder
 *
 *  (C) Copyright 2006 Denis Rojo <jaromil@dyne.org>
 *                     Joan & Dirk <jodi@jodi.org>
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

#ifndef __TIMEBASEDTEXT_H__
#define __TIMEBASEDTEXT_H__


#include <time.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <pthread.h>

#include <linklist.h>

#define VERSION "v0.3 - by Jaromil & Jodi"


/*
  This is the format of a single entry:

  int key  - S-Lang key number (matches ASCII/ANSI)
  int sec  - time delta seconds
  int usec - time delta 1/100 seconds */
class TBTEntry : public Entry {
 public:
  TBTEntry();
  ~TBTEntry();

  uint64_t key;
  uint64_t msec;

  /* parse from *buf and return true on success */
  bool parse_uint64(void *buf);
  
  /* render in *buf and returns size in bytes */
  int render_uint64(void *buf);
  int render_ascii(void *buf);
  int render_javascript(void *buf);

};

/*
  Clock timer abstraction
  using RTC in Linux/BSD
  multithreaded
 */
class TBTClock {
 public:
  TBTClock();
  ~TBTClock();

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
    ((TBTClock *) arg)->run();
    return NULL;
  };
  
};

class TBT {
  
 public:
  TBT();
  ~TBT();

  int init();

  void append(uint64_t key);

  void clear(); ///< deletes all keys and frees memory

  uint64_t getkey(); ///< wait the time and returns the next entry
  int position; ///< incremented by getkey calls

  int load(char *filename);

  int save_bin(char *filename);
  int save_ascii(char *filename);
  int save_javascript(char *filename);

  Linklist buffer;

  TBTClock clock;

 private:

  void compute_delta(TBTEntry *tbt);

  uint64_t now;
  uint64_t past;

  bool rtc; // if /dev/rtc is present

  // POSIX time structures
  struct timespec psleep; // nanosleep (nanosec)
  struct timeval gettime; // gettimeofday (microsec)

};

#endif
