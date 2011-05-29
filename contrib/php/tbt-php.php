<?
#
#     ooooooooooo oooooooooo  ooooooooooo
#     88  888  88  888    888 88  888  88
#         888      888oooo88      888
#         888      888    888     888
#        o888o    o888ooo888     o888o
#
# (C) Copyright 2006 - 2007  Angelo Failla <pallotron@freaknet.org>
#     based on the tbt C++ code produced by Denis Rojo <jaromil@dyne.org>
#     Idea shared with Joan & Dirk <jodi@jodi.org>
#
# This is a PHP4/5 compatible class that allows webmasters to convert a 
# .tbt binary file, produced using the tbt software # (http://tbt.dyne.org) 
# into a string to be used with the TBT javascript class coded by 
# jaromil@dyne.org 
#
# In the .tbt format, text is saved with a timestamp for each letter:
#
#   _________ ___________
#  |  u-int  |   u-int   |
#  |  64bit  |   64bit   |    m-seconds = 1/1000 of a second
#  --------- -----------
#  |  char   | m-seconds |    char = ASCII or higher bit char code
#  --------- -----------
#  |    Total: 128bit    |
#  ---------------------
# 
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU Public License as published 
# by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
# 
# This source code is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# Please refer to the GNU Public License for more details.
# 
# You should have received a copy of the GNU Public License along with
# this source code; if not, write to:
# Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# 
# HOW TO USE
# ==========
#
# include_once("/path/to/tbt-php.php");
# $TBT = new TBT_JSRender("/path/to/your/record.tbt");
# if($TBT->is_error) {
# 	print $TBT->get_error()."\n";
# 	die();
# } 
# $string = $TBT->get_jsstr(); 
# $txt = $TBT->get_text();
#
# now you can pass $string to the TBT javascript class
# $txt will contain only the text

class TBT_JSRender {

	var $jsstr;
	var $is_error;
	var $error_msg;

	// constructor
	// it loads a tbt file, parses it, and 
	// fills the $jsstr string
	function TBT_JSRender($filename) {
		
		settype($msec,"integer");
		$this->is_error=false;

		if (! file_exists($filename)) {
			$this->is_error=true;
			$this->error_msg="$filename: no such file or directory!";
			return false;
		}

		if (! $this->is_tbtfile($filename)) {
			$this->is_error=true;
			$this->error_msg="$filename: this is not a valid tbt file!";
			return false;
		}

		$handle = fopen($filename, "rb");
		if(!$handle) {
			$this->is_error=true;
			$this->error_msg="$filename: unable to open file!";
			return false;
		}
		$this->jsstr = "[";
		while(! feof($handle)) {
			// the character value
			$bin  = fread($handle, 8);
			$data = @unpack("L1x", $bin);
			$this->jsstr .= "[".$data["x"].",";
			// the timeout value
			$bin  = fread($handle,8);
			$data = @unpack("L1x",$bin);
			$this->jsstr .= $data["x"]."],";
		}
		$this->jsstr .= "]";
		$this->jsstr  = str_replace("[,]","", $this->jsstr);
		$this->jsstr  = str_replace(",,]","]", $this->jsstr);
		fclose($handle);

		return true;

	}

	// return the string to be used with the JS typewriter
	function get_jsstr() {
		return $this->jsstr;
	}

	// this method returns only the text of the tbt file
	// any backspace is processed deleting the previous
	// character
	//
	// a string like "palll^Hotrooo^H^Hn"
	// will be returned as "pallotron"
	//
	function get_text() {
		$array = split(",", $this->jsstr);
		$array2 = array();
		$string="";
		$i=0;
		foreach($array as $a) {
			if( ($i + 1) % 2 != 0 ) {
				$int = str_replace("[","",$a);
				$array2[]=$int;
			}
			$i++;
		}
		for($i=0;$i<sizeof($array2);$i++) {
			if($array2[$i] == 272) {
				$tmp = array_slice($array2, 0, $i-1);
				$tmp2 = array_slice($array2, $i+1);
				$array2 = array_merge($tmp, $tmp2);
				$i=0;
			}
		}
		foreach($array2 as $b) {
			if($b == 13) $string.="\n";
			else $string.=chr($b);
		}
		return $string;
	}

	// this is a simple check to see if 
	// the provided file is a tbt file
	// it will simply check if the filesize
	// is a multiple of 16 bytes (aka 128bits)
	function is_tbtfile($filename) {
		$size = filesize($filename);
		if( $size % 16 != 0) return false; 
		else return true;
	}

	function get_error() {
		return $this->error_msg;
	}

}	

?>
