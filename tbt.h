/*  Time Based Text - Recorder
 *
 *  (C) Copyright 2006 - 2007 Denis Rojo <jaromil@dyne.org>
 *      Idea shared with Joan & Dirk <jodi@jodi.org>
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
#include <sys/time.h>

#include <linklist.h>

#define VERSION "v0.9 - tbt.dyne.org"


class RTClock; // ghost pointer for rtclock.h


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
  /* parse from an ascii line and return true on success */
  bool parse_ascii(char *buf);
  
  /* render in *buf and returns size in bytes */
  int render_uint64(void *buf);
  int render_ascii(void *buf);
  int render_html(void *buf);

};


class TBT {
  
 public:
  TBT();
  ~TBT();

  int init();

  // input functions: time based append of keys into a tbt recording

  void append(uint64_t key); ///< append a single key at the time this function is called

  int fdappend(int filedes, int keysize); ///< append keys read from a file descriptor




  // playback functions: time based feed of keys loaded

  uint64_t getkey(); ///< wait the time and returns the next entry

  int position; ///< incremented by getkey calls



  // loading functions: load a tbt recording for playback

  int load(char *filename); ///< load a .tbt recorded file
  //TODO:
  // int load_html(char *filename);
  int load_ascii(char *filename);


  void clear(); ///< deletes all current keys and frees memory  

  int save_bin(char *filename);
  int save_ascii(char *filename);
  int save_html(char *filename);

  Linklist *buffer;

  bool rtc; // if /dev/rtc is present
  RTClock *clock;

  bool quit;


 private:

  void compute_delta(TBTEntry *tbt);

  uint64_t now;
  uint64_t past;



  // POSIX time structures
  struct timespec psleep; // nanosleep (nanosec)
  struct timeval gettime; // gettimeofday (microsec)

};

#endif
