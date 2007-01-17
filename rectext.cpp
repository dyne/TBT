/*  Time Based Text - Commandline Recorder
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


// Time Based Text
#include <tbt.h>



// S-Lang widgets
#include <slw_console.h>
#include <slw_text.h>


// my lovely utils
#include <jutils.h>



// Time Based Text global object
TBT tbt;



///////////////// commandline stuff

void set_status(SLangWidget *s);

static const char *help =
"Usage: rectext [options] [file]\n"
"\n"
"  -h   print this help\n"
"  -v   version information\n"
"  -D   debug verbosity level - default 1\n"
"  -r   render format (bin|ascii|javascript)\n"
"  -i   record keys from stdin (byte size)\n";
// "  -w   word wrap on screen bounds\n"
// "  -s   scrolling text\n"

static const char *short_options = "-hvD:r:i:";

int debug;
char filename[512];

// rendering formats
#define BIN        1
#define ASCII      2
#define JS         3
int render = BIN;

int stdinput = 0;

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
	      
    case 'i':
      stdinput = atoi(optarg);
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

  for(;optind<argc;optind++)
    snprintf(filename, 511, "%s", argv[optind]);

  if(!filename[0])
    sprintf(filename, "record.tbt");

  set_debug(debug);

}

///////////// end of commandline stuff



int record_console() {
  // S-Lang console
  SLangConsole con;
  
  // S-Lang widgets
  SLW_Text txt;
  SLW_Text status;

  uint64_t key;


  // initialize the text console
  if(! con.init() ) return(-1);
  
  //  initialize the status line
  status.border = false;
  status.set_name("status box");
  if(! con.place(&status, 0, con.h-10, con.w, con.h) ) {
    error("error placing the status widget");
    return(-1);
  }
  assert ( status.init() );
  
  //  set the status widget *** only after placing it! ***
  set_status(&status);
  
  // initialize the text canvas
  txt.set_name("editor");
  // txt.border = true;
  if(! con.place(&txt, 0, 0, con.w, con.h-6) ) { //  con.w, con.h-20) ) {
    error("error placing the text widget");
    return(-1);
  }
  assert ( txt.init() );
  
  // focus the text canvas
  con.focused = &txt;
  
  
  // write out to the status widget
  notice("TBT - console ready");
  
  while(!tbt.quit) {
    
    key = con.getkey();
    
    if(key) {
      
      // save the key and timestamp
      tbt.append(key);
      
      // display the key
      con.feed(key);
      
    }
    
    if( ! con.refresh() ) tbt.quit = true;
    
    jsleep(0,10);
    
  }

  // cleanly close the console
  con.refresh();
  
  jsleep(1,0);
  con.close();

  return 1;
}





int main(int argc, char** argv)
{
 
  cmdline(argc, argv);

  
  // start the TBT engine
  if(! tbt.init() )  exit(0);

  
  // check if recording from stdin
  if(stdinput) {

    // call the recorder for the stdin file descriptor
    tbt.fdappend(0, stdinput);
    
  } else { // recording from s-lang console

    record_console();
    
  }
  
  notice("Closing Time Based Recorder");
    
  if( render == BIN ) {
    
    tbt.save_bin( filename );
    act("TBT file %s rendered in binary format",filename);
    
  } else if( render == ASCII ) {
    
    tbt.save_ascii( filename );
    act("TBT file %s rendered in binary format",filename);
    
  } else if( render == JS ) {
    
    tbt.save_javascript( filename );
    act("TBT file %s rendered in binary format",filename);
    
  }
  

  exit(0);  
}
