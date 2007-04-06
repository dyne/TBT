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

function msieversion() {
	var ua = window.navigator.userAgent
	var msie = ua.indexOf ( "MSIE " )

	if ( msie > 0 )      // If Internet Explorer, return version number
		return parseInt (ua.substring (msie+5, ua.indexOf (".", msie )))
	else                 // If another browser, return 0
		return 0
}

function MultiDimensionalArray(iRows,iCols) {
	var i;
	var j;
	var a = new Array(iRows);
	
	for (i=0; i < iRows; i++) { 
		a[i] = new Array(iCols); 
		for (j=0; j < iCols; j++) { 
			a[i][j] = ""; 
		} 
	}
	return(a);
} 

function is_collision(i) {
	var j
	
	for(j=0;j<=i;j++) {
		if( ( (ran_x[i] > ran_x[j])             && (ran_x[i]             < ran_x[j] + width[j]) ) ||
			( (ran_y[i] > ran_y[j])             && (ran_y[i]             < ran_y[j] + height[j]) )  ||
			( (ran_x[i] + width[i]  > ran_x[j]) && (ran_x[i] + width[i]  < ran_x[j] + width[j] ) ) ||
			( (ran_y[i] + height[i] > ran_y[j]) && (ran_y[i] + height[i] < ran_y[j] + height[j]) ) ) return true;
	}
	return false;
}

function print_divs() {

	for(i=0; i<tbt_array.length; i++) {

		ran_y[i] = Math.floor(Math.random()*(max_y-min_y))+min_y
		ran_x[i] = Math.floor(Math.random()*(max_x-min_x))+min_x
		var str = '<div id="div_'+i+'" style="vertical-align: middle; background-color: #333; opacity: .5; '
		if(msieversion()<7 && msieversion()!=0) str += 'filter:alpha(opacity=50);'
		str += ' border: 1px solid; visibility: hidden; z-index: '+(i+1)+
			'; position: absolute; left:'+ran_x[i]+'px; top:'+ran_y[i]+ 'px;"><img style="cursor:move;" src="images/hand.png"/>'+
			'<span class="andalemono" onclick="playTBT(tbt_array['+i+']);"> <a href="#">'+tbt_array[i]['title']+'</a></span></div>'
		document.getElementById("res").innerHTML += str
		if(msieversion()<7 && msieversion()!=0) correctPNG()
		width[i]  = document.getElementById("div_"+i).clientWidth
		height[i] = document.getElementById("div_"+i).clientHeight
		if(i>=1) {
			while(is_collision(i)) {
				ran_y[i] = Math.floor(Math.random()*(max_y-min_y))+min_y
				ran_x[i] = Math.floor(Math.random()*(max_x-min_x))+min_x
			}
		}
		document.getElementById("div_"+i).style.left = ran_x[i]
		document.getElementById("div_"+i).style.top  = ran_y[i]
		document.getElementById("div_"+i).style.visibility = "visible"
		if(i+1 == max_result) break;

	}

}

function playTBT(tbt_array) {

	if(total_counter % 3 == 0) {
		row_counter = 0
	}

	document.getElementById("tbt_2").innerHTML = document.getElementById("tbt_1").innerHTML
	document.getElementById("tbt_1").innerHTML = document.getElementById("tbt_0").innerHTML

	var related_string = ""
	var related_array  = ""
	var xmlrequest = null
	request = CreateXmlHttpReq2()
	request.open("GET","xml_tbt_info.php?action=related&id="+tbt_array['id'],false);
	request.send(null);
	if (request.readyState == 4 && request.status == 200) {
		var response = request.responseXML.documentElement;
		x=response.getElementsByTagName("num")
		num = x[0].firstChild.data
		if(num>0) {
			x=response.getElementsByTagName("tbt")
			for(i=0;i<x.length;i++) {
				var id = x[i].getElementsByTagName("id")[0].firstChild.data
				var author   = x[i].getElementsByTagName("author")[0].firstChild.data
				var title    = x[i].getElementsByTagName("title")[0].firstChild.data
				var email    = x[i].getElementsByTagName("email")[0].firstChild.data
				var city     = x[i].getElementsByTagName("city")[0].firstChild.data
				var datetime = x[i].getElementsByTagName("datetime")[0].firstChild.data
				var tbt      = x[i].getElementsByTagName("data")[0].firstChild.data
				related_string += "<span onclick='playTBT_noarray("+
					id+",\""+author+"\",\""+title+"\",\""+email+"\",\""+city+"\",\""+datetime+"\","+tbt+
					")'><a href='#'>"+title+" by "+author+"</a></span><br/>"
			}
		} else related_string = "nothing"
	}

	document.getElementById("tbt_"+row_counter).innerHTML = 
	"<div style='border-top: 1px solid black; border-left: 1px solid black; border-right: 1px solid black; padding: 5px 5px 5px 5px; background-color:"+
	bgcolorlist[Math.floor(Math.random()*bgcolorlist.length)]+";'"+
	"class='couriersmall'><b>From: "+tbt_array['author']+" <br/>&lt;<a href=\"mailto:"+tbt_array['email']+"\">"+tbt_array['email']+"</a>&gt;<br/>"+
	"Title: "+tbt_array['title']+"<br/>"+
	"Date/Time: "+tbt_array['datetime']+"<br/>"+
	"City: "+tbt_array['city']+"<br/></b>"+
	"</div><div id='tbttext_"+row_counter+
	"' style='border-left: 1px solid black; border-top: 1px solid black; border-right: 1px solid black; padding: 5px 5px 5px 5px; background-color:"+
	bgcolorlist[Math.floor(Math.random()*bgcolorlist.length)]+ ";' class='couriersmall'></div>"+
	"<div style='border: 1px solid black; padding: 5px 5px 5px 5px; background-color:"+bgcolorlist[Math.floor(Math.random()*bgcolorlist.length)]+";'"+
	"class='couriersmall'>Related TBT's: <br/>"+related_string+"</div>"

	var tbt = new TBT()
	tbt.startTyping("tbttext_"+row_counter, tbt_array['tbt'])

	total_counter++
}

function playTBT_noarray(id,author,title,email,city,datetime,tbt) {

	var tbt_arr = new Array(7)

		tbt_arr['id'] = id
		tbt_arr['author'] = author
		tbt_arr['title'] = title
		tbt_arr['email'] = email
		tbt_arr['city'] = city
		tbt_arr['datetime'] = datetime
		tbt_arr['tbt'] = tbt

		playTBT(tbt_arr)
}

function printMsg(what) {

	if(total_counter % 3 == 0) {
		row_counter = 0
	}

	document.getElementById("tbt_2").innerHTML = document.getElementById("tbt_1").innerHTML
	document.getElementById("tbt_1").innerHTML = document.getElementById("tbt_0").innerHTML

	document.getElementById("tbt_"+row_counter).innerHTML=
		"</div><div id='tbttext_"+row_counter+
		"' style='border: 1px solid black; padding: 5px 5px 5px 5px; background-color:"+bgcolorlist[Math.floor(Math.random()*bgcolorlist.length)]+
		";' class='couriersmall'></div>"

	var tbt = new TBT()
	tbt.startTyping("tbttext_"+row_counter, what) 

	total_counter++
}

