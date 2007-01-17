/*  Time Based Text - Player reference implementation
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



// Time Based Text object
TBT tbt;


////// commandline options stuff

void set_status(SLangWidget *s);

static const char *help =
"Usage: playtext [options] [file]\n"
"\n"
"  -h   print this help\n"
"  -c   playback in a text console\n"
"  -v   version information\n"
"  -D   debug verbosity level - default 1\n";
// "  -w   word wrap on screen bounds\n"
// "  -s   scrolling text\n"

static const char *short_options = "-hvD:";

int debug;
char filename[512];
bool console = false;

void cmdline(int argc, char **argv) {
  int res;

  debug = 1;
  filename[0] = 0x0;

  do {
    res = getopt(argc, argv, short_options);
    switch(res) {

    case 'h':
      fprintf(stderr, "playtext:\t");
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

    case 'c':
      console = true;
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

  for(;optind<argc;optind++)
    snprintf(filename, 511, "%s", argv[optind]);

  if(!filename[0])
    sprintf(filename, "record.tbt");

  set_debug(debug);

}

//////////// end of commandline option stuff

// our global console
SLangConsole *con;



int play_console() {
  
  // our widgets
  SLW_Text txt;
  SLW_Text status;
  
  con = new SLangConsole();

  // initialize the text console
  if(! con->init() ) exit(-1);

  //  initialize the status line
  status.border = false;
  status.set_name("status box");
  if(! con->place(&status, 0, con->h-10, con->w, con->h) ) {
    error("error placing the status widget");
    return(-1);
  }
  assert ( status.init() );
  
  //  set the status widget *** only after placing it! ***
  set_status(&status);
 
  // place the text canvas
  if(! con->place(&txt, 0, 0, con->w, con->h-1) ) {
	  error("error placing the text widget");
	  return(-1);
  }
  txt.set_name("player");
  assert ( txt.init() );
  // focus the text console
  con->focused = &txt;
  
  return 1;
}



int main(int argc, char **argv) {
  uint64_t key;
  int c, len;

  cmdline(argc, argv);

  len = tbt.load( filename );

  if(!len) {
    fprintf(stderr,"no entries found in file %s\n",argv[1]);
    exit(1);
  }

  ///////////////////////
  // start the TBT engine
  tbt.init();


  // initialize the console if needed
  if(console) play_console();



  // main playback loop
  for(c=0; c<len && !tbt.quit; c++) {

    // tbt.getkey is a blocking call
    // will wait N time before returning
    key = tbt.getkey();

    if(console) {

      con->feed(key);
      if( ! con->refresh() ) tbt.quit = true;

    } else {
      
      // print out on stdout
      write(1, (void*)&key, 1);

    }

  }

  if(console) con->close();
  exit(0);
}
