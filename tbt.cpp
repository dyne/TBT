/*  Time Based Text
 *
 *  (C) Copyright 2006 - 2008  Denis Roio <jaromil@dyne.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>

#include <tbt.h>
#include <jutils.h>
#include <string.h>
#include <errno.h>

#include <slw/keycodes.h>

#ifdef linux
#include <rtclock.h>
#endif


TBTEntry::TBTEntry() 
 : Entry() {
  key  = 0;
  msec = 0;
  // just a hint
  set_name("char64,msec64");
}

TBTEntry::~TBTEntry() { }

bool TBTEntry::parse_uint64(void *buf) {

  uint64_t *p = (uint64_t*)buf;

  key  = *p;
  msec = *(p+1);
  
  return true;
}


// the  following method  takes an  ascii line  (readed from  an ascii
// file) and convert it to the key,msec couple.

// there's  actually a problem  with the  backspace key...  the S-Lang
// widget  prints  ^P...  without  delete  the  previous  char...  the
// tbtchk_ascii.cpp test  problem print correctly  the backspace value
// in my shell... :( - pallotron 08/04/2007

// found out with caedes: standard ascii code for BS is 8
// somehow s-lang reports different values when grabbing low level
// kind of weird to see how many different values are on various platforms...
// anyway we'll normalize all this to ASCII (dec 8) now. -jrml

bool TBTEntry::parse_ascii(char *buf) {

  unsigned char k = 0;
  unsigned long int m = 0;

  sscanf(buf, "%c:%lu\n", &k, &m);
  key = (uint64_t)k;
  msec = (uint64_t)m;

  return true;
}


int TBTEntry::render_uint64(void *buf) {
  int len; // length in bytes
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

int TBTEntry::render_html(void *buf) {
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





TBT::TBT()  {

  // zeroing counters
  now    = 0;
  past   = 0;

  position = 1;

  rtc  = false;
  quit = false;

  // posix timing
  gettime.tv_sec   = 0;
  gettime.tv_usec  = 0;
  psleep.tv_sec    = 0;
  psleep.tv_nsec   = 0;

  buffer = new Linklist;

  clock  = NULL;

}

TBT::~TBT() {

  clear();

  delete buffer;
}

int TBT::init() {

#ifdef linux
  if(rtc) { // realtime clock use requested
    rtc = false; // let's check now

    // try /dev/rtc clock
    clock = new RTClock();
    if( clock->init() ) {
      notice("real time clock device is present");
      
      
      // set the frequency to 1024 HZ
      // so 1 second we have 1024 ticks (what we call microsec)
      // it is very hard to match machine time with human time, indeed
      // if you care about *real* precision, then this is the riddle:
      // /dev/rtc allows to set as frequencies only powers of two.
      clock->set_freq( 1024 );
      
      // this launches the internal clock thread
      if( ! clock->start() ) {
	error("no permission to run real time clock at 1024HZ");
	delete clock;
	clock = NULL;
      } else
	rtc = true;
    }
  }
#endif
  
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
  ent = (TBTEntry*)buffer->begin();
  
  // when deleting objects that's the trick with linklist
  // it scrolls up the next at the beginning
  while(ent) {
    delete ent;	
    ent = (TBTEntry*) buffer->begin();
  }
  
}

void TBT::append(uint64_t key) {
  TBTEntry *ent;
  ent = new TBTEntry();

  // compute time delay since last key
  // the NOW is WHEN this operation is executed
  compute_delta( ent );

  // normalize ambiguous keycodes
  switch( key ) {

  case KEY_BACKSPACE_ASCII:
  case KEY_BACKSPACE:
  case KEY_BACKSPACE_APPLE:
  case KEY_BACKSPACE_SOMETIMES:
    ent->key = (uint64_t)KEY_BACKSPACE_ASCII;
    break;
    
  case KEY_NEWLINE:
  case KEY_ENTER:
    ent->key  = (uint64_t)KEY_ENTER;
    break;

  default:
    ent->key = (uint64_t)key;

  }

  buffer->append(ent);
}





uint64_t TBT::getkey() {
  TBTEntry *ent;
  ent = (TBTEntry*) buffer->pick(position);
  
  if(!ent) {
    func("NULL entry at position %u", position);
    return 0;
  }

  position++;

  // this pauses execution by time delta in entry
  // here are implemented rtc and posix
  if(!rtc) {

    psleep.tv_sec = ent->msec / 1000;
    psleep.tv_nsec = (ent->msec % 1000) * 1000000;
    nanosleep(&psleep, NULL);

  }
#ifdef linux
    else clock->sleep(ent->msec);
#endif

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
    if(!len) break;
    else if(len != 4) {
      warning("truncated entry, %u elements read", len);
      continue;
    }
    
    ent = new TBTEntry();

    if( ! ent->parse_uint64(buf) ) {
      error("error in TBTEntry::parse_uint64");
      continue;
    }

    buffer->append( ent );
    c++;

  }

  free(buf);

  position = 1;

  return c;
}

// pallotron: 08/04/2007
// this method is the same of the load() method.
// except that it reads from a pure ascii file
// each line is passed to the parse_ascii method
int TBT::load_ascii(char *filename) {

  int c;

  char ascii_line[512];

  FILE *fd;

  fd = fopen(filename, "r");
  if(!fd) {
    error("can't open ascii file: %s", strerror(errno));
    return false;
  }

  clear();

  TBTEntry *ent;

  c = 0;
  
  act("loading ascii file %s", filename);

  while( ! feof(fd) ) {

    fgets(ascii_line, 512, fd);
    
    ent = new TBTEntry();

    if( ! ent->parse_ascii(ascii_line) ) {
      error("error in TBTEntry::parse_ascii");
      continue;
    }

    buffer->append( ent );
    c++;

  }

  position = 1;

  return c;
}

int TBT::save_bin(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "w");
  if(!fd) {
    error("can't oper file %s for writing: %s", filename, strerror(errno));
    return false;
  }
  
  // max bytes for an entry here
  buf = malloc(512);


  TBTEntry *ent;
  ent = (TBTEntry*) buffer->begin();

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
  ent = (TBTEntry*)buffer->begin();

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

int TBT::save_html(char *filename) {

  int c, len;

  void *buf;

  FILE *fd;

  fd = fopen(filename, "w");
  if(!fd) return false;

  fputs("<html><head>\n"
	"<script language=\"JavaScript\" src=\"tbt-typewriter.js\"></script>\n"
	"<title>Time Based Text - recorded with tbt.dyne.org</title></head>\n"
	"<body text=\"#FFFFFF\" bgcolor=\"#000000\">\n"
	"<div id=\"textDestination\">\n"
	"</div>\n"
	"<script language=\"JavaScript\">\n"
	"<!--\n", fd);

  // start array
  fwrite("var TimeBasedText=[",sizeof(char),19,fd);

  // max bytes for an entry here
  buf = malloc(512);

  TBTEntry *ent;
  ent = (TBTEntry*) buffer->begin();

  // cycle thru with counter
  c = 0;
  while( ent ) {

    if(c>0) // put a comma
      fwrite(",",sizeof(char),1,fd);

    len = ent->render_html(buf);

    fwrite(buf, len, 1, fd);
    
    c++;
	
    ent = (TBTEntry*) ent->next;
  }


  // close array
  fwrite("]\n",sizeof(char),2,fd);

  fputs("\n"
	"var tbt = new TBT();\n"
	"tbt.startTyping(\"textDestination\", TimeBasedText);\n"
	"//--></script>\n"
	"</body></html>\n\n", fd);
  fflush(fd);
  fclose(fd);

  free(buf);

  return c;
}

void TBT::compute_delta(TBTEntry *tbt) {

  // get the NOW
  // here is implemented rtc and posix
  if(!rtc) {

    // posix 1003.1-2001 gettimeofday
    if( gettimeofday(&gettime, NULL) <0)
      error("error getting time: %s",strerror(errno));
    now = (gettime.tv_sec * 1000) + (gettime.tv_usec / 1000);
    
  }
#ifdef linux
    else now = clock->msec;
#endif

  tbt->msec = now - past;
  // take care of unsigned

  past = now;

}
