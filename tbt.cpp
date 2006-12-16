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


#include <tbt.h>
#include <jutils.h>


// threading stuff
typedef void* (kickoff)(void*);


TBTEntry::TBTEntry() 
 : Entry() {
  key  = 0;
  sec  = 0;
  usec = 0;
  // just a hint
  set_name("char64/sec32/usec32");
}

TBTEntry::~TBTEntry() { }

bool TBTEntry::parse_uint64(void *buf) {

  uint32_t *p = (uint32_t*)buf;

  key  = *(uint64_t*)(p);
  sec  = *(p+2);
  usec = *(p+3);
  
  return true;
}


int TBTEntry::render_uint64(void *buf) {
  int len;
  uint32_t tmp[4];

  ((uint64_t*)tmp)[0] = key;
  tmp[2] = sec;
  tmp[3] = usec;

  len = 16;

  memcpy(buf, (void*)tmp, len);

  return len;
}

int TBTEntry::render_ascii(void *buf) {
  int len;
  char tmp[512];
  
  len = snprintf(tmp,511,"%c:%u,%u\n",(int)key, sec, usec);
  if(len<0) {
    error("error rendering in ascii: %s", strerror(errno));
    return 0;
  }

  memcpy(buf,tmp,len*sizeof(char));
  return len;
}

int TBTEntry::render_javascript(void *buf) {
  int len;
  char tmp[512];

  len = snprintf(tmp,511,"[%u,%u,%u]", (int)key, sec, usec);
  if(len<0) {
    error("error rendering in javascript array: %s", strerror(errno));
    return 0;
  }

  memcpy(buf, tmp, len*sizeof(char));
  return len;
}  



TBTClock::TBTClock() {
  rtcfd = 0;
  quit = false;
  sec = 0;
  microsec = 0;
}

int TBTClock::init() {
  int res;

  // initialize thread and rtc
  if(pthread_attr_init (&_attr) == -1)
    error("error initializing POSIX thread attribute");
  
  /* sets the thread as detached
     see: man pthread_attr_init(3) */
  //  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_DETACHED);
  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_JOINABLE);

  // open RTC device
  rtcfd = ::open("/dev/rtc",O_RDONLY);
  if(rtcfd<0) {
    error("couldn't open real time clock device: %s", strerror(errno));
    return -1;
  }
  /* set the alarm event to 1 second */
  res = ioctl(rtcfd, RTC_UIE_ON, 0);
  if(res<0) {
    error("couldn't set real time clock device tick: %s", strerror(errno));
  }
  act("real time clock succesfully initialized");
  
  return(rtcfd);
}

int TBTClock::close() {
  // close rtc
  if(rtcfd<=0) return(0);
  ioctl(rtcfd, RTC_UIE_OFF, 0);
  //  ioctl(rtcfd,RTC_PIE_OFF,0);
  ::close(rtcfd);
  
  // close posix threads
  //  if(pthread_attr_destroy(&_attr) == -1)
  //  error("error destroying POSIX thread attribute");

  return rtcfd;
}
  

TBTClock::~TBTClock() {
  // close rtc
  if(rtcfd>0) close();

  // close posix threads
  if(pthread_attr_destroy(&_attr) == -1)
    error("error destroying POSIX thread attribute");

}


int TBTClock::set_freq(unsigned long freq) {
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
  sec = 0;
  microsec = 0;

  return res;
}



void TBTClock::run() {
  int res;
  
  // run the clock
  while(!quit) {

    res = read(rtcfd, &rtctime, sizeof(unsigned long));
    if(res < 0)
      error("problem catching real time clock interrupt: %s", strerror(errno));

    
    if(microsec > 1000) {
      sec++;
      microsec = 0;

      act("clock sec %lu", sec); 

    } else microsec++;



  }

}



int TBTClock::start() {
  return pthread_create(&_thread, &_attr, &kickoff, this);
}


/* Macros for converting between `struct timeval' and `struct timespec'.
#define TIMEVAL_TO_TIMESPEC(tv, ts) {                   \
        (ts).tv_sec =  (tv).tv_sec;                     \
        (ts).tv_nsec = (tv).tv_usec * 1000;             \
}
#define TIMESPEC_TO_TIMEVAL(tv, ts) {                   \
        (tv).tv_sec =  (ts).tv_sec;                     \
        (tv).tv_usec = (ts).tv_nsec / 1000;             \
}
they seem to be already included in sys/time.h
*/


/* Macro to find time delta between now and past timeval */
#define DELTA_TIMEVAL(n, p, d) {                        \
        (d).tv_sec  = (n).tv_sec  - (p).tv_sec;         \
        (d).tv_usec = (n).tv_usec - (p).tv_usec;        \
}  


TBT::TBT()  {
  // zeroing counters
  now.tv_sec    = 0;
  now.tv_sec    = 0;
  past.tv_sec   = 0;
  past.tv_usec  = 0;
  delta.tv_sec  = 0;
  delta.tv_usec = 0;

  position = 1;

  if( gettimeofday(&past, NULL) <0 ) {
    error("error getting time of day: %s", strerror(errno));
    exit(1);
  }
}

TBT::~TBT() {
  clear();
}

void TBT::clear() {
  // erase all entries and free the memory
  TBTEntry *ent;
  ent = (TBTEntry*)buffer.begin();

  // when deleting objects that's the trick with linklist
  // it scrolls up the next at the beginning
  while(ent) {
        delete ent;	
	ent = (TBTEntry*) buffer.begin();
  }
}

void TBT::append(uint64_t key) {
  TBTEntry *ent;
  ent = new TBTEntry();
  
  if( gettimeofday(&now, NULL) <0 )
    error("error getting time: %s",strerror(errno));

  // compute time delay since last key
  DELTA_TIMEVAL(now, past, delta);

  // store current time in the past for next delta
  memcpy(&past, &now, sizeof(struct timeval));

  ent->key  = (uint64_t)key;
  ent->sec  = (uint32_t)delta.tv_sec;
  ent->usec = (uint32_t)delta.tv_usec;

  buffer.append(ent);
}

uint64_t TBT::getkey() {
  TBTEntry *ent;
  ent = (TBTEntry*) buffer[position];

  if(!ent) {
	  func("NULL entry at position %u", position);
	  return 0;
  }

  timespec nsleep;

  position++;

  nsleep.tv_sec = ent->sec;
  // here convert micro to nano seconds
  nsleep.tv_nsec = ent->usec *1000;
  
  act("sleeping %u secs and %u microsec",ent->sec, ent->usec);

  // perform the nanosleep
  nanosleep(&nsleep, NULL);
//  while ( nanosleep (&nsleep, &rem) == -1 && (errno == EINTR) );

  return ent->key;
}

int TBT::load(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "r");
  if(!fd) {
    error("can't open file: %s", strerror(errno));
    return false;
  }

  clear();

  // max bytes for an entry here
  buf = malloc(64);

  TBTEntry *ent;

  c = 0;
  
  act("loading file %s", filename);

  while( ! feof(fd) ) {

    len = fread(buf, 4, 4, fd);

    if(len != 4) {
      warning("truncated entry, %u elements read", len);
      continue;
    }
    
    ent = new TBTEntry();

    if( ! ent->parse_uint64(buf) ) {
      error("error in TBTEntry::parse_uint64");
      continue;
    }

    buffer.append( ent );
    c++;

  }

  free(buf);

  position = 1;

  return c;
}


int TBT::save_bin(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "w");
  if(!fd) return false;
  
  // max bytes for an entry here
  buf = malloc(512);


  TBTEntry *ent;
  ent = (TBTEntry*) buffer.begin();

  // cycle thru with counter
  c = 0;
  while( ent ) {
 
    len = ent->render_uint64(buf);

    fwrite(buf, len, 1, fd);
    
    c++;
	
    ent = (TBTEntry*) ent->next;

  }

  
  fflush(fd);
  fclose(fd);

  free(buf);

  return c;
}

int TBT::save_ascii(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "w");
  if(!fd) return false;
  
  // max bytes for an entry here
  buf = malloc(512);

  TBTEntry *ent;
  ent = (TBTEntry*)buffer.begin();

  // cycle thru with counter
  c = 0;
  while( ent ) {
 
    len = ent->render_ascii(buf);

    fwrite(buf, len, 1, fd);
    
    c++;

    ent = (TBTEntry*) ent->next;
  }
  
  fflush(fd);
  fclose(fd);

  free(buf);

  return c;
}

int TBT::save_javascript(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "w");
  if(!fd) return false;
  
  // start array
  fwrite("var TimeBasedText=[",sizeof(char),19,fd);

  // max bytes for an entry here
  buf = malloc(512);

  TBTEntry *ent;
  ent = (TBTEntry*) buffer.begin();

  // cycle thru with counter
  c = 0;
  while( ent ) {

    if(c>0) // put a comma
      fwrite(",",sizeof(char),1,fd);

    len = ent->render_javascript(buf);

    fwrite(buf, len, 1, fd);
    
    c++;
	
    ent = (TBTEntry*) ent->next;
  }


  // close array
  fwrite("]\n",sizeof(char),2,fd);

  fflush(fd);
  fclose(fd);

  free(buf);

  return c;
}
