/*  Time Based Text - Recorder
 *
 *  (C) Copyright 2006 - 2012 Denis Roio <jaromil@dyne.org>
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

/** 
    @file tbt.h TBT main header
    @brief this is the main header to use the TBT API
*/

#ifndef __TIMEBASEDTEXT_H__
#define __TIMEBASEDTEXT_H__


#include <time.h>
#include <inttypes.h>
#include <sys/time.h>

#include <linklist.h>

class RTClock; // ghost pointer for rtclock.h

class TBTEntry; // defined later


/**
   @class TBT
   @brief Time Based Text player and recorder class

This  class offers  an  easy programming  interface  to implement  TBT
applicatons that playback, convert and record new message sequences.

It works as  a memory buffer holding the time  based text: for example
it can load TBT files, edit them adding and removing entries, save the
result  into a  new  file.   Its high-level  API  includes methods  to
read/write to files of various formats (ascii, binary or html)

TBT also offers access  to the lower-lever interface, providing access
to the  actual sequence of entries collected:  it's member TBT::buffer
is a Linklist  where TBTEntry units can be  directly accessed.

*/

class TBT {
  
 public:
  TBT();
  ~TBT();

  /**
     @defgroup TBTAPI High level API

     Main TBT functionalities are provided by a few intuitive methods,
     easy to embed in object oriented software.

     The high  level API takes  care of timing, calculating  the delta
     for  entries at the  time the  rec/play methods  (TBT::append and
     TBT::getkey)  are called.   If  an application  requires to  gain
     control over timing  calculation then it is necessary  to use the
     low level api.

     @{
  */

  int init(); ///< this should be called before starting use of TBT


  // input functions: time based append of keys into a tbt recording

  void append(uint64_t key); ///< append a key marked at the time this function is called
  uint64_t getkey(); ///< wait the time delta of the key and then returns it
  int position; ///< current position incremented by getkey calls


  // loading functions: load a tbt recording for playback

  int load(char *filename); ///< load a .tbt binary file
  int save_bin(char *filename); ///< save a .tbt file in binary format


  int load_ascii(char *filename); ///< load a .tbt ASCII file
  int save_ascii(char *filename); ///< save a .tbt ASCII file, in plain text format

  //TODO: int load_html(char *filename);
  int save_html(char *filename); ///< save a .html page suitable for web browsers

  int save_doku(char *filename); ///< save a dokuwiki page


  void clear(); ///< deletes all current keys and frees memory  

  /// @}

  Linklist *buffer; ///< the linklist of all TBTEntry:: currently loaded or recorded

  bool quit; ///< quit switch (facility for running loops)

  bool rtc;
  RTClock *clock;


 private:

  void compute_delta(TBTEntry *tbt);
  ///< synchronous function to compute the time delta of an entry at the time of being called

  uint64_t now;  ///< time register to compute delta
  uint64_t past; ///< time register to compute delta

  struct timespec psleep; ///< POSIX time structure - nanosleep (nanosec)
  struct timeval gettime; ///< POSIX time structure - gettimeofday (microsec)

};


/**
   @class TBTEntry
   @brief A single time based entry

Every  single entry  (keypress) is  implemented as  described  by this
class: containing  a key identifier  TBTEntry::key and the  time delta
for the entry  TBTEntry::msec (the time one should  wait before giving
the identifier).

You don't need to use this class: for high-level interaction the TBT::
class  provides all what  is needed  to record  and playback,  using a
linklist of TBTEntry:: . But in  case you want low-level access to the
data itself then you are looking in the right place.

Keep in  mind that  input (parsing) via  single entry methods  in this
class is  time independent: it won't  save time of  input, instead the
higher level  API provided by  TBT:: will save  the time delta  at the
moment when parsing methods are invoked.

*/

class TBTEntry : public Entry {
 public:
  TBTEntry();
  ~TBTEntry();

  /**
     @defgroup TBTEntry_data Low level API - entry data section

     The following  properties hold the entry identifier  and the time
     delta; this  data can  be accessed directly  (r/w) or  parsed and
     rendered from different formats using the helper_methods.

     @{
  */
  uint64_t key; ///< key identifier for the entry, usually ASCII/ANSI/UTF-8
  uint64_t msec; ///< time delta for the entry, noted as 1/100 seconds

  /// @}

  /**
     @defgroup TBTEntry_methods Low level API - entry parsing and rendering

     Helper methods  are provided for  every single entry to  read and
     write values from and to different formats.

     @{
  */

  
  bool parse_uint64(void *buf); ///< parse an entry from a 128 bit buffer: [(64)key,(64)time]
  bool parse_ascii(char *buf); ///< parse an entry from a colon separated ASCII string (key:time)
  
  int render_uint64(void *buf); ///< render the entry into a 128 bit buffer
  int render_ascii(void *buf); ///< render the entry into an ASCII string (colon separated)
  int render_html(void *buf); ///< render the entry into HTML (javascript array)

  /// @}
};


#endif
