/*  Time Based Text - Player reference implementation
 *
 *  (C) Copyright 2006 Denis Rojo <jaromil@dyne.org>
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

#include <getopt.h>

#include <assert.h>

// S-Lang widgets
#include <slw_console.h>
#include <slw_text.h>

#include <tbt.h>

#include <jutils.h>
void set_status(SLangWidget *s);


static const char *help =
"Usage: playtext [options] [file]\n"
"\n"
"  -h   print this help\n"
"  -v   version information\n"
"  -D   debug verbosity level - default 1\n";
// "  -w   word wrap on screen bounds\n"
// "  -s   scrolling text\n"

static const char *short_options = "-hvD:";

int debug;
char filename[512];


void cmdline(int argc, char **argv) {
  int res;

  debug = 1;
  filename[0] = 0x0;

  do {
    res = getopt(argc, argv, short_options);
    switch(res) {

    case 'h':
      fprintf(stderr, "playtext:\t", argv[0]);
      fprintf(stderr, VERSION);
      fprintf(stderr, "%s", help);
      exit(0);
      break;

    case 'v':
      fprintf(stderr,"\n");
      exit(0);
      break;

    case 'D':
      debug = atoi(optarg);
      if(debug>3) {
	warning("debug verbosity ranges from 1 to 3\n");
	debug = 3;
      }
      break;

    case 1:
      {
	FILE *fd;
	snprintf(filename,511,"%s", optarg);
	fd = fopen(filename, "r");
	if(!fd) {
	  error("can't read %s", filename);
	  exit(1);
	} else fclose(fd);
      }
      
    default: break;
      
    }
  } while (res != -1);

#ifdef HAVE_BSD
  for(;optind<argc;optind++)
    snprintf(filename, 511, "%s", argv[optind]);
#endif

  if(!filename[0])
    sprintf(filename, "/tmp/record.tbt");

  set_debug(debug);

}


// our global console
SLangConsole con;

// our widgets
SLW_Text txt;
SLW_Text status;

// Time Based Text object
TBT tbt;


int main(int argc, char **argv) {
  uint64_t key;
  int c, len;

  cmdline(argc, argv);

  len = tbt.load( filename );

  if(!len) {
    fprintf(stderr,"no entries found in file %s\n",argv[1]);
    exit(1);
  }

  // initialize the text console
  if(! con.init() ) exit(-1);

  //  initialize the status line
  status.border = false;
  status.set_name("status box");
  if(! con.place(&status, 0, con.h-1, con.w, con.h) ) {
    error("error placing the status widget");
    exit(-1);
  }
  assert ( status.init() );
  
  //  set the status widget *** only after placing it! ***
  set_status(&status);
 
  // place the text canvas
  if(! con.place(&txt, 0, 0, con.w, con.h-1) ) {
	  error("error placing the text widget");
	  exit(-1);
  }
  txt.set_name("player");
  assert ( txt.init() );
  // focus the text console
  con.focused = &txt;

  bool quit = false;

  for(c=0; c<len && !quit; c++) {
    // this is a blocking call
    // tbt.getkey will wait N time before returning
    key = tbt.getkey();

    con.feed(key);
    
    if( ! con.refresh() ) quit = true;
  }

  con.close();

  exit(0);
}
