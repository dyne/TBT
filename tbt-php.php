<?
	class TBT_JSRender {

		var $js_array;

		// constructor
		// load a tbt file into memory
		function TBT_JSRender($filename) {

			settype($msec,"integer");

			if (! file_exists($filename)) {
				return -1;
			}


			$handle = fopen($filename, "rb");
			$jstr="[";
			while(! feof($handle)) {
				// the character value
				$bin= fread($handle, 8);
				$data=unpack("V1x",$bin);
				$jstr.="[".$data["x"].",";
				// the timeout value
				$bin= fread($handle,8);
				$data=unpack("V1x",$bin);
				$jstr.=$data["x"]."],";
			}
			$jstr.="]";
			str_replace("],","]",$jstr);
			print $jstr;
			fclose($handle);

			return 0;

		}

	}	

	$TBT = new TBT_JSRender("/home/pallotron/dynecvs/tbt/record.tbt");
?>
