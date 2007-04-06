/*  Time Based Text - web application
 *
 *  (C) Copyright 2006 - 2007  Angelo Failla <pallotron@freaknet.org>
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
 */

// Create the XML HTTP request object. We try to be
// more cross-browser as possible.
function CreateXmlHttpReq2() {
	xmlhttp=null
	// code for Mozilla, etc.
	if (window.XMLHttpRequest) {
  		xmlhttp=new XMLHttpRequest()
  	}
	// code for IE
	else if (window.ActiveXObject) {
  		xmlhttp=new ActiveXObject("Microsoft.XMLHTTP")
  	}
	if (xmlhttp==null) {
  		alert("Your browser does not support XMLHTTP.")
  	}
	return xmlhttp;
}
