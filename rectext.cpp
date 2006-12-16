/*  Time Based Text - Recorder
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


//// system includes
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include <getopt.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>

#include <assert.h>


//////////////////////

// S-Lang widgets
#include <slw_console.h>
#include <slw_text.h>
// #include <slw_popup.h>

#include <tbt.h> // Time Based Text

#include <jutils.h> // my lovely utils
void set_status(SLangWidget *s);

static const char *help =
"Usage: rectext [options] [file]\n"
"\n"
"  -h   print this help\n"
"  -v   version information\n"
"  -D   debug verbosity level - default 1\n"
"  -r   render format (bin|ascii|javascript)\n";
// "  -w   word wrap on screen bounds\n"
// "  -s   scrolling text\n"

static const char *short_options = "-hvD:r:";

int debug;
char filename[512];

// rendering formats
#define BIN        1
#define ASCII      2
#define JS         3
int render = BIN;

void cmdline(int argc, char **argv) {
  int res;

  debug = 1;
  filename[0] = 0x0;

  fprintf(stderr, "TBT - Time Based Text - recorder\n");
  fprintf(stderr, "      %s\n", VERSION);

  do {
    res = getopt(argc, argv, short_options);
    switch(res) {
    case 'h':
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

    case 'r':
      if( strncasecmp(optarg, "BIN", 3) ==0)
	      render = BIN;
      else if( strncasecmp(optarg, "ASCII", 5) ==0)
	      render = ASCII;
      else if( strncasecmp(optarg, "JAVASCRIPT", 10) ==0)
	      render = JS;
      else {
	      error ("render format not recognized: %s", optarg);
	      act ("using default binary format render");
	      render = BIN;
      }
      break;
	      

    case 1:
      {
	FILE *fd;
	snprintf(filename,511,"%s", optarg);
	fd = fopen(filename, "w");
	if(!fd) {
	  error("can't write in %s: %s", filename, strerror(errno));
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
//SLW_Popup popup;
 
// Time Based Text object
TBT tbt;
TBTClock tbtclock;

int main(int argc, char** argv)
{
  int key;
  bool quit = false;
 
  cmdline(argc, argv);

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
  

  // initialize the popup
  /*  popup.set_name("popup");
      popup.set_text("this is a test of a popup\n"
      "it goes on multiple lines hopefully\n"
      "without breaking the whole screen\n"
      "the termination is a null, here we go.");
      if(! con.place(&popup, 20, 1, con.w-10, con.h-30) ) {
      error("error placing the popup widget");
      exit(-1);
      }
      assert (popup.init() );
  */
  

  // initialize the text canvas
  txt.set_name("editor");
  // txt.border = true;
  if(! con.place(&txt, 0, 0, con.w, con.h-6) ) { //  con.w, con.h-20) ) {
	  error("error placing the text widget");
	  exit(-1);
  }
  assert ( txt.init() );

  // focus the text canvas
  con.focused = &txt;
 
  // start the clock
  //  tbtclock.start();

  // write out to the status widget
  notice("TBT - console ready");

  while(!quit) {

    key = con.getkey();

    if(key) {

      // save the key and timestamp
      tbt.append(key);

      // display the key
      con.feed(key);

      }
    
    if( ! con.refresh() ) quit = true;

    jsleep(0,10);

  }

  notice("Save and quit");

  if( render == BIN ) {
	  
  	tbt.save_bin( filename );
  	act("file %s rendered in binary format",filename);

  } else if( render == ASCII ) {

  	tbt.save_ascii( filename );
  	act("file %s rendered in binary format",filename);

  } else if( render == JS ) {

  	tbt.save_javascript( filename );
   	act("file %s rendered in binary format",filename);
 
  }

  con.refresh();

  jsleep(1,0);
  con.close();

  exit(0);  
}
