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
