// TBT javascript prototype: Jaromil 2007
// reference javascript typewriter documentation: F. Permadi 2002
// GNU GPL



function TBT() { };
    

function type()	{
    var dest = document.getElementById(destination);
    
    if (dest)// && dest.innerHTML)
	{
	    /* TODO: handle movement keys:
	       #define KEY_BACKSPACE 275
	       #define KEY_BACKSPACE_APPLE 127
	       #define KEY_BACKSPACE_SOMETIMES 272
	       #define KEY_UP 257
	       #define KEY_DOWN 258
	       #define KEY_LEFT 259
	       #define KEY_RIGHT 260
	       #define KEY_PAGE_UP 261
	       #define KEY_PAGE_DOWN 262
	       #define KEY_HOME 263
	       #define KEY_DELETE 275
	       #define KEY_TAB 9
	       port the abstract_console from slw in js
	    */
	    
	    switch( recording[currentChar][0] ) {
	    case 13: // carriage return
		this.text += "<br>";
		break;
	    case 32: // white space
		this.text += "&nbsp;";
		break;
	    default:
		this.text += String.fromCharCode( recording[currentChar][0] );
	    }
	    dest.innerHTML= this.text + "_";
	    // DOM compliant alternative:
	    //	var textNode=document.createTextNode(text);
	    //	dest.replaceChild(textNode, dest.childNodes[0]);
	    
	    
	    
	    // go to the next entry
	    currentChar++;
	    
	    
	    if (currentChar >= recording.length) {
		
		// end of record, restart after 5 seconds
		currentChar=0;
		text="";
		setTimeout("type()", 5000 + recording[0][1]);
		
	    } else {
		
		setTimeout("type()", recording[currentChar][1] );
		
	    }
	}
}
TBT.prototype.type = type;

    
function startTyping(destinationParam, tbtrecord) {
    currentChar = 0;
    destination = destinationParam;
    recording   = tbtrecord;
    text = " ";
    setTimeout("type()", recording[currentChar][1] );
}
TBT.prototype.startTyping = startTyping;
