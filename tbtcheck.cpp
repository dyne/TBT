/*  Time Based Text - .tbt file checker
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

#include <tbt.h>
#include <keycodes.h>

#include <jutils.h>

// Time Based Text object
TBT tbt;

int main(int argc, char **argv) {
  int c, len;

  len = tbt.load( argv[1] );
  if(!len) {
    fprintf(stderr,"no entries found in file %s\n",argv[1]);
    exit(1);
  }

  TBTEntry *stroke;
  char keyname[256];

  for(c=1; c<len+1; c++) {
    // this is a blocking call
    // tbt.getkey will wait N time before returning
    stroke =  (TBTEntry*) tbt.buffer[c];
    switch(stroke->key) {
      case KEY_ENTER:   sprintf(keyname,"ENTER"); break;
      case KEY_UP:      sprintf(keyname,"UP");    break;
      case KEY_DOWN:    sprintf(keyname,"DOWN");  break;
      case KEY_LEFT:    sprintf(keyname,"LEFT");  break;
      case KEY_RIGHT:   sprintf(keyname,"RIGHT"); break;
      default: sprintf(keyname, "%c", stroke->key);
    }
    act("[ %s ] - %us %ums", keyname, stroke->sec, stroke->usec);
    
    
  }

  exit(0);

}




