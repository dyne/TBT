/*  Time Based Text - Commandline Player and Recorder
 *
 *  (C) Copyright 2006 - 2008 Denis Roio <jaromil@dyne.org>
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
#include <termios.h>
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
#include <signal.h>

//////////////////////


// Time Based Text
#include <tbt.h>



// S-Lang widgets
#include <slw_console.h>
#include <slw_text.h>
#include <slw_popup.h>

// my lovely utils
#include <jutils.h>



// Time Based Text global object
TBT tbt;



///////////////// commandline stuff

void set_status(SLangWidget *s);

static const char *help =
"Synopsis: tbt <command> [options] [file]\n"
"Commands:\n"
"  -r   record\n"
"  -p   playback\n"
"  -x   convert\n"
"  -v   print version\n"
"  -h   print this help\n"
"Options:\n"
"  -C   stdin/out commandline mode\n"
"  -s   save format in [ bin | ascii | html | doku ]\n"
#ifdef linux
"  -t   timing mode    [ posix | rtc ]\n"
#endif
"  -D   debug verbosity level (1-3)\n";





static const char *short_options = "-hvD:crpxs:t:";

int debug;
char filename[512];
char convert[512];


// act as commandline tool by default
bool console = true;

// timing modes
#define POSIX 1
#define RTC   2
int timing = POSIX;

// operation modes
int operation = 0x0;
#define REC        1
#define PLAY       2
#define CONV       4


// rendering formats
#define BIN        1
#define ASCII      2
#define HTML       3
#define DOKU       4
int render = BIN;


uint64_t key;

int c, len;


void cmdline(int argc, char **argv) {
  int res;

  debug = 1;
  filename[0] = 0x0;

  fprintf(stderr, "TBT - Time Based Text - %s\n", VERSION);
  fprintf(stderr, "Software by Jaromil @ Dyne.org\n\n");

  /// adjust the operation to the way the binary is called
  if ( ! strstr(argv[0], "tbt") )
    console = true; // use console by default when binary is aliased
  if      ( strstr(argv[0],"rectext") )
    operation = REC;
  else if ( strstr(argv[0],"playtext") )
    operation = PLAY;


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

    case 'C':
      console = false;
      break;

    case 'c': /* deprecated */
      console = true;
      break;

    case 'r':
      operation = REC;
      break;

    case 'p':
      operation = PLAY;
      break;

    case 'x':
      operation = CONV;
      break;

    case 't':
      if(strncasecmp(optarg, "rtc", 3) ==0)
	timing = RTC;
      else
	timing = POSIX;
      break;

    case 's':
      if( strcasecmp(optarg, "BIN") ==0)
	      render = BIN;
      else if( strcasecmp(optarg, "ASCII") ==0)
	      render = ASCII;
      else if( strcasecmp(optarg, "HTML") ==0)
	      render = HTML;
      else if( strncasecmp(optarg, "DOKU", 4) ==0)
	      render = DOKU;
      else {
	      error ("render format not recognized: %s", optarg);
	      act ("using default binary format render");
	      render = BIN;
      }
      break;

      
    case 1:  snprintf(filename,511,"%s", optarg);
      
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




//// console based operations using S-Lang interface

// S-Lang console
SLangConsole *con;

// S-Lang widgets
SLW_Text     *txt;
SLW_Text     *status;

int record_console() {
  // initialize the text console
  assert( con->init() );

  // give a welcome popup before starting recording
  SLW_Popup *info;
  info = new SLW_Popup();
  info->border = false;
  info->set_color(1);
  info->set_name("welcome popup");
  info->set_text("Welcome to Time Based Text\n"
		 "press [ENTER] to start recording\n"
		 "press [ESC] when finished.");
  con->place(info,
	     (con->w/2) - 20, (con->h/2) - 5,
	     (con->w/2) + 20, (con->h/2) + 5);
  assert( info->init() );
  con->focused = info;
  key = 0;
  while( con->refresh() ) {
    key = con->getkey();
    if(key == KEY_ENTER) break;
    if(key == KEY_ESC) return 0;
  }
  delete(info);
  
  //  initialize the status line
  status->border = false;
  status->set_color(21);
  status->set_name("status box");

  con->place(status, 0, con->h-1, con->w, con->h);

  assert ( status->init() );
  
  //  set the status widget *** only after placing it! ***
  set_status(status);
  
  // initialize the text canvas
  txt->set_name("editor");
  txt->cursor = true;
  // txt->border = true;
  if(! con->place(txt, 0, 0, con->w, con->h-2) ) { //  con->w, con->h-20) ) {
    error("error placing the text widget");
    return(-1);
  }
  assert ( txt->init() );
  
  // focus the text canvas
  con->focused = txt;
  txt->blank();
  
  // write out to the status widget
  notice("TBT recording, press [ESC] when finished");
  
  while(!tbt.quit) {
    
    key = con->getkey();

    if(key == KEY_ESC) break;

    if(key) {
      
      // save the key and timestamp
      tbt.append(key);
      
      // display the key
      con->feed(key);
      
    }
    
    if( ! con->refresh() ) tbt.quit = true;
    
    jsleep(0,10);
    
  }

  return 1;
}


int play_console() {
  

  //  initialize the status line
  status->border = false;
  status->set_name("status box");
  if(! con->place(status, 0, con->h-10, con->w, con->h) ) {
    error("error placing the status widget");
    return(-1);
  }
  assert ( status->init() );
  
  //  set the status widget *** only after placing it! ***
  set_status(status);
 
  // place the text canvas
  if(! con->place(txt, 0, 0, con->w, con->h-1) ) {
	  error("error placing the text widget");
	  return(-1);
  }
  txt->set_name("player");
  assert ( txt->init() );
  // focus the text console
  con->focused = txt;



  // main playback loop
  for(c=0; c<len && !tbt.quit; c++) {
    
    // tbt.getkey is a blocking call
    // will wait N time before returning
    key = tbt.getkey();

    con->feed(key);
    if( ! con->refresh() ) tbt.quit = true;

  }
  
  return 1;
}

//////////////////////////////////// end of console based operations


/* signal handling */
void quitproc (int Sig) {
  act("interrupt caught, exiting.");
  tbt.quit = true;
}



int main(int argc, char** argv)
{

  cmdline(argc, argv);

  if(!operation) {
    error("no operation specified, use -h for help on usage.");
    exit(0);
  }

  if(timing == RTC)
    tbt.rtc = true;

  // start the TBT engine
  if(! tbt.init() )  exit(0);

  /* register signal traps */
  if (signal (SIGINT, quitproc) == SIG_ERR) {
    perror ("Couldn't install SIGINT handler"); exit(0); }

  // if playing, load the recording
  if(operation == PLAY) {
    if( ! tbt.load( filename ) ) {
      error("no entries found in file %s",filename);
      exit(1);
    } else
      len = tbt.buffer->len();
  }

  if(console) { // initialize the s-lang console
    
    con = new SLangConsole();
    
    // initialize the text console
    if(! con->init() ) {
      error("cannot initialize S-Lang console on this terminal.");
      exit(0);
    }
    
    txt    = new SLW_Text();
    status = new SLW_Text();
    
    switch(operation) {
    
    case REC:
      if( !record_console() ) break;
      
      switch(render) {
	
      case BIN:
	tbt.save_bin( filename );
	break;
	
      case ASCII:
	tbt.save_ascii( filename );
	break;
	
      case DOKU:
	tbt.save_doku( filename );
	break;

      case HTML:
	tbt.save_html( filename );
	break;
	
      }
      
      break;
      
    case PLAY:
      play_console();
      break;
      
    }
    
    // cleanly close the console
    con->refresh();
    jsleep(1,0);
    con->close();
    set_status(NULL);

    if(operation == REC)
      notice("TBT file %s rendered in %s format", filename,
	  (render==BIN)?"binary":(render==ASCII)?"ascii":(render==HTML)?"html":"unknown");


  } else {
    ////////////// commandline operations
    
    switch(operation){
      
    case REC:

      {
	// OMFG
	struct termios stty;
	tcgetattr(0,&stty);
	stty.c_lflag ^= ICANON|ECHO;
	stty.c_cc[VTIME] = 100; // 10 seconds minute timeout 
	stty.c_cc[VMIN] = 0;
	tcsetattr(0,TCSANOW,&stty);

	while(!tbt.quit) {
	  
	  read(0,(void*)&key,1);

	  tbt.append((int64_t)key);

	  write(1, (void*)&key, 1);	  

	}

	stty.c_lflag |= ICANON|ECHO;
	tcsetattr(0,TCSANOW,&stty);
      }

      switch(render) {
	
      case BIN:
	tbt.save_bin( filename );
	break;
	
      case ASCII:
	tbt.save_ascii( filename );
	break;
	
      case HTML:
	tbt.save_html( filename );
	break;
	
      case DOKU:
	tbt.save_doku( filename );
	break;
      }
      
      break;
      
    case PLAY:
      
      // main playback loop
      for(c=0; c<len && !tbt.quit; c++) {
	// tbt.getkey is a blocking call
	// will wait N time before returning
	key = tbt.getkey();

	// print out on stdout
	write(1, (void*)&key, 1);
      }
      break;




    case CONV:
      // convert to other formats
      tbt.load(filename);
      
      switch(render) {
	
      case ASCII:
	{

	  snprintf(convert, 511, "%s.asc",filename);	
	  tbt.save_ascii( convert );
	  
	}
	break;
	
      case HTML:
	{
	  snprintf(convert, 511,"%s.html",filename);	
	  tbt.save_html( convert );
	}
	break;
      case BIN:
	  snprintf(convert, 511,"%s.tbt",filename);	
	  tbt.save_bin( convert );
      }
    }
  }    

  if( (operation==REC) | (operation==CONV) )
    notice("TBT file %s %s to %s format",
	   (operation==REC)?filename:convert,
	   (operation==REC)?"recorded":
	   (operation==CONV)?"converted":"***",
	   (render==BIN)?"binary":
	   (render==ASCII)?"ascii":
	   (render==HTML)?"html":
	   (render==DOKU)?"dokuwiki":"unknown");
    
  act("exiting TBT");
  act("%u entries streamed.", tbt.buffer->len());

  exit(0);  
}
