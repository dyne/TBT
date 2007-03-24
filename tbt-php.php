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
# if($TBT->is_error == TBT_NO_SUCH_FILE) {
# 	print "File doesn't exist\n";
# 	die();
# } 
# if($TBT->is_error == TBT_FILE_NOT_VALID) {
# 	print "Problably this is not a valid tbt file!\n";
# 	print "File size isn't multiple of 16 bytes!\n";
# 	die();
# }
# $string = $TBT->get_jsstr(); 
#
# now you can pass $string to the TBT javascript class

define("TBT_NO_SUCH_FILE",   "-1");
define("TBT_FILE_NOT_VALID", "-2");

class TBT_JSRender {

	var $jsstr;
	var $is_error;

	// constructor
	// it loads a tbt file, parses it, and 
	// fills the $jsstr string
	function TBT_JSRender($filename) {
		
		settype($msec,"integer");

		if (! file_exists($filename)) {
			$this->is_error=TBT_NO_SUCH_FILE;
			return $this->is_error;
		}

		if (! $this->is_tbtfile($filename)) {
			$this->is_error=TBT_FILE_NOT_VALID;
			return $this->is_error;
		}

		$handle = fopen($filename, "rb");
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

		return 0;

	}

	function get_jsstr() {
		return $this->jsstr;
	}

	function is_tbtfile($filename) {
		$size = filesize($filename);
		if( $size % 16 != 0) return 0; 
		else return 1;
	}

}	

?>
