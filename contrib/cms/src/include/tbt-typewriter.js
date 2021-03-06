// TBT javascript prototype: Jaromil 2007
// reference javascript typewriter documentation: F. Permadi 2002
// GNU GPL



function TBT() { };

    
function startTyping(destinationParam, tbtrecord) {
    currentChar = 0;
    destination = destinationParam;
    recording   = tbtrecord;

    // global text positions
    row = 0;
    col = 0;
    // cursor positions
    cur_x = 0;
    cur_y = 0;
    // length counters
    rowsize = 0;
    colsize = 0;
    cr = 0;
    cc = 0;
    
    // text bidimensional array
    text = new Array();
    text[0] = new Array();
    // text render buffer
    render_text = "";    

    setTimeout("feed()", recording[currentChar][1] );
}
TBT.prototype.startTyping = startTyping;

function feed()	{
    var dest = document.getElementById(destination);
    
    if (dest) {
	// && dest.innerHTML)
	
	/* TODO: handle movement keys:
	   KEY_BACKSPACE 275, 127, 272
	   KEY_PAGE_UP 261 ; KEY_PAGE_DOWN 262
	   KEY_HOME 263 ; KEY_DELETE 275
	   port the abstract_console from slw in js
	*/
	
	// get the next char
	switch( recording[currentChar][0] ) {
	case 13:  // RETURN
	    cur_y++; row++;
	    cur_x = 0; col = 0;
	    if(row >= text.length) {
		text[row] = new Array();
	    }
	    break;

	case 127: // BACKSPACE
	case 272: // =
	case 275: // =

	    if(col > 0) // delete if not at the beginning of line
		text[row].splice(col-1,1);
	    
	    if(cur_x > 0) {
		cur_x--;
		col--;
	    } else { // backspace at the beginning of a line
		     // move everything up one line
		// TODO
	    }
	    break;

	case 257: // UP
	    if(row <= 0) break;
	    cur_y--; row--;
	    break;
	case 258: // DOWN
	    if(row >= text.length) break;
	    cur_y++; row++;
	    break;
	case 259: // LEFT
	    if(cur_x <= 0) break;
	    cur_x--;
	    col--;
	    break;
	case 260: // RIGHT
	    //	    if(cur_x >= text[row].length) break;
	    cur_x++;
	    col++;
	    break;

	default:
	    text[row].splice(col,0,recording[currentChar][0]);
	    cur_x++; col++;
	    break;
	}

	// RENDER TEXT
	render_text = "";

	rowsize = text.length;
	for(cr = 0; cr < rowsize; cr++) {
	    
	    colsize = text[cr].length;

	    if(cur_y == cr) { // cursor row

		for(cc = 0; cc < colsize; cc++) {
		    
		    if(cur_x == cc) { // draw cursor
			    render_text += "<u>";
			    render_text += String.fromCharCode( text[cr][cc] );
			    render_text += "</u>";
		    } else // not yet on cursor
			render_text += String.fromCharCode( text[cr][cc] );

		}
		
		// draw cursor at the end of line
		if(cur_x >= colsize) render_text += "_";
		
	    } else  // no need to draw the cursor
		for(cc = 0; cc < colsize; cc++)
		    render_text += String.fromCharCode( text[cr][cc] );


	    // newline
	    render_text += "<br>";
	}

	// render_text += String.fromCharCode( recording[currentChar][0] );
	dest.innerHTML = render_text;
	


	// DOM compliant alternative:
	//	var textNode=document.createTextNode(text);
	//	dest.replaceChild(textNode, dest.childNodes[0]);
	
	
	
	// go to the next entry
	currentChar++;
	
	
	if (currentChar >= recording.length) {
	    
	    // end of record, restart after 5 seconds
	    currentChar=0;
	    render_text="";

	    // clear all text buffer
	    for(cr=0; cr < text.length; cr++)
		delete text[cr];
	    delete text;
		//at the end make text link
		//newLink = "<a href='link'>"+document.getElementById("textDestination").innerHTML+"</a>";
		//document.getElementById("textDestination").innerHTML = newLink;
            // end of text
	    
	} else {
	    
           // RECURSION IS TIME  -jrml 31jan2007
	    setTimeout("feed()", recording[currentChar][1] );
	    
	}
    }
}
TBT.prototype.feed = feed;

