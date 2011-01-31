/*  Time Based Text - Mail Recorder
 *
 *  (C) Copyright 2006 Denis Rojo <jaromil@dyne.org>
 *
 * This is a text console editor for mails, it opens a text file
 * including mail headers and adds the Attach: of the .tbt recording
 * it is being developed for use as editor of mutt.
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
"  -t   include plain text in the mail\n";

static const char *short_options = "-hvD:r:";

int debug;
char filename[512];
bool plaintext = false;

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

    case 't':
      plaintext = true;
      break;
	      

    case 1:
      {
	FILE *fd;
	snprintf(filename,511,"%s", optarg);
	fd = fopen(filename, "r+");
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

int main(int argc, char** argv)
{
  int key;
  bool quit = false;
  FILE *mailfd;
  char tbtfile[512];

  cmdline(argc, argv);

  if(!filename[0]) {
    error("no file supplied on the commandline, see usage instructions");
    exit(0);
  }

  func("composing mail in %s",filename);

  // initialize the text console
  if(! con.init() ) exit(-1);

  //  initialize the status line
  status.border = false;
  status.set_name("status box");
  if(! con.place(&status, 0, con.h-10, con.w, con.h) ) {
    error("error placing the status widget");
    exit(-1);
  }
  assert ( status.init() );
  
  //  set the status widget *** only after placing it! ***
  set_status(&status);
  

  // initialize the text canvas
  txt.set_name("editor");
  // txt.border = true;
  if(! con.place(&txt, 0, 0, con.w, con.h-11) ) { //  con.w, con.h-20) ) {
	  error("error placing the text widget");
	  exit(-1);
  }
  assert ( txt.init() );

  // focus the text canvas
  con.focused = &txt;
 

  // start the TBT engine
  if(! tbt.init() ) {
    con.close();
    error("fatal error: can't start the TBT engine");
    exit(0);
  }

  // write out to the status widget
  notice("TBT - console recording - press Ctrl-c when done ");



  // open message file read-write
  mailfd = fopen(filename,"r+");
  if(!mailfd)
    error("can't open file %s: %s", filename, strerror(errno) );
  else {

    // parse the headers from the input file
    char hdrp[1024];
    long pos;

    // go to the last header
    while( fgets(hdrp, 1024, mailfd) ) {
      if(hdrp[0] == '\n') {
	// final part fund
	pos = ftell(mailfd);
	fseek(mailfd, pos-1, SEEK_SET);
	break;
      }
      func("parsed header - %s", hdrp);
    }

    // append the attach header to the message
    snprintf(tbtfile,511,"Attach: %s.tbt\n",filename);
    fputs(tbtfile, mailfd);
    fclose(mailfd);

    // form the filename.tbt
    snprintf(tbtfile,511,"%s.tbt", filename);
  }
      
  while(!quit) {

    key = con.getkey();

    if(key) {

      // save the key and timestamp
      tbt.append(key);

      // display the key
      con.feed(key);

      }
    
    if( ! con.refresh() ) quit = true;

    jsleep(0,1);

  }

  notice("Save and quit");

  act("rendering %s in binary format", tbtfile);
  tbt.save_bin( tbtfile );

  con.refresh();

  con.close();

  exit(0);  
}
