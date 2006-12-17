/*  Time Based Text
 *
 *  (C) Copyright 2006 Denis Rojo <jaromil@dyne.org>
 *    Idea shared with Joan & Dirk <jodi@jodi.org>
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
  msec = 0;
  // just a hint
  set_name("char64/sec32/msec32");
}

TBTEntry::~TBTEntry() { }

bool TBTEntry::parse_uint64(void *buf) {

  uint64_t *p = (uint64_t*)buf;

  key  = *p;
  msec = *(p+1);
  
  return true;
}


int TBTEntry::render_uint64(void *buf) {
  int len;
  uint64_t tmp[2];

  tmp[0] = key;
  tmp[1] = msec;

  len = 16; //sizeof(uint64_t)*2;

  memcpy(buf, (void*)tmp, len);

  return len;
}

int TBTEntry::render_ascii(void *buf) {
  int len;
  char tmp[512];
  
  len = snprintf(tmp,511,"%c:%lu\n",(int)key, (unsigned long)msec);
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

  len = snprintf(tmp,511,"[%lu,%lu]", (unsigned long)key, (unsigned long)msec);
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
  msec = 0;
  _thread = 0x0;
  sleeping = false;
}


TBTClock::~TBTClock() {

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

int TBTClock::init() {
  int res;

  // open RTC device
  rtcfd = ::open("/dev/rtc",O_RDONLY);
  if(rtcfd<0) {
    error("couldn't open real time clock device: %s", strerror(errno));
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
  msec = 0;

  return res;
}

int TBTClock::tick() {
  int res;

  // blocking read on /dev/rtc
  res = read(rtcfd, &rtctime, sizeof(unsigned long));
  if(res < 0)
    error("problem catching real time clock interrupt: %s", strerror(errno));

  // update clock counter
  msec++;
  
  return res;
}

void TBTClock::run() {

  // run the clock
  while(!quit) {

    if(sleeping) wait();
    
    tick(); // blocking
    
  }

}



int TBTClock::start() {
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

int TBTClock::sleep(uint64_t msec) {
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

TBT::TBT()  {

  // zeroing counters
  now    = 0;
  past   = 0;

  position = 1;

  rtc = false;

  // posix timing
  gettime.tv_sec   = 0;
  gettime.tv_usec  = 0;
  psleep.tv_sec    = 0;
  psleep.tv_nsec   = 0;

}

TBT::~TBT() {
  clear();
}

int TBT::init() {
  if( clock.init() ) {
    notice("real time clock device is present");

    
    // set the frequency to 1024 HZ
    // so 1 second we have 1024 ticks (what we call microsec)
    // it is very hard to match machine time with human time, indeed
    // if you care about *real* precision, then this is the riddle:
    // /dev/rtc allows to set as frequencies only powers of two.
    clock.set_freq( 1024 );
    
    // this launches the internal clock thread
    if( ! clock.start() )
      error("can't run real time clock at 1024HZ");
    else
      rtc = true;

  }

  if(!rtc) { // use posix

    if( gettimeofday(&gettime, NULL) <0 ) {
      error("error getting (posix) time: %s",strerror(errno));
      error("no timing source available, impossible to go further");
      return 0;
    }

    past = (gettime.tv_sec * 1000) + (gettime.tv_usec / 1000);
  
  }

  return 1;
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

  // compute time delay since last key
  // the NOW is WHEN this operation is executed
  compute_delta( ent );

  ent->key  = (uint64_t)key;

  buffer.append(ent);
}

uint64_t TBT::getkey() {
  TBTEntry *ent;
  ent = (TBTEntry*) buffer[position];
  
  if(!ent) {
    func("NULL entry at position %u", position);
    return 0;
  }

  position++;

  // this pauses execution by time delta in entry
  // here are implemented rtc and posix
  if(rtc)

    clock.sleep(ent->msec);

  else {

    psleep.tv_sec = ent->msec / 1000;
    psleep.tv_nsec = (ent->msec % 1000) * 1000000;
    nanosleep(&psleep, NULL);

  }

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

void TBT::compute_delta(TBTEntry *tbt) {

  // get the NOW
  // here is implemented rtc and posix
  if(rtc)

    now = clock.msec;

  else { // posix 1003.1-2001 gettimeofday

    if( gettimeofday(&gettime, NULL) <0)
      error("error getting time: %s",strerror(errno));
    now = (gettime.tv_sec * 1000) + (gettime.tv_usec / 1000);
    
  }

  tbt->msec = now - past;
  // take care of unsigned

  past = now;

}
