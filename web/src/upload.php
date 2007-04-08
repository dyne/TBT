<?
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
?>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>

	<title>TBT - Upload TBT</title>
	<style type="text/css" media="all">
		@import "styles/default.css";
	</style>
	<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">

</head>

<?
	include_once("include/config.inc.php");
	include_once("include/db.inc.php");
	include_once("include/tbt-php.php");
	include_once("include/third_part/captcha/captcha.php");
	$action = $_GET['action'];
	$relatedto = $_GET['reactionto'];
	if($relatedto == "") $relatedto=0;
	else {
		$react_title = $_GET['react_title'];
		$react_author = $_GET['react_author'];
	}
?>

<body class="popupbody">

<script language="Javascript">

	function checkFields() {
		var title = document.upload.title.value;
		var name  = document.upload.name.value;
		var city  = document.upload.city.value;
		var email = document.upload.email.value;
		var file  = document.upload.filename.value;
		var captcha_input = document.upload.captcha_input.value;
		var filter=/^([\w-]+(?:\.[\w-]+)*)@((?:[\w-]+\.)*\w[\w-]{0,66})\.([a-z]{2,6}(?:\.[a-z]{2})?)$/i

		if ((title == "") || (title == "undefined")) {
			alert("Title field is mandatory.");
			document.upload.title.style.background="#ff3333";
			document.upload.title.focus();
			return false;
		} else document.upload.title.style.background=""
		
		if ((name == "") || (name == "undefined")) {
			alert("Name field is mandatory.");
			document.upload.name.style.background="#ff3333";
			document.upload.name.focus();
			return false;
		} else document.upload.name.style.background=""

		if ((file == "") || (file == "undefined")) {
			alert("You must select a file to upload");
			document.upload.filename.style.background="#ff3333";
			document.upload.filename.focus();
			return false;
		} else document.upload.filename.style.background=""


		if ((city == "") || (city == "undefined")) {
			alert("City field is mandatory.");
			document.upload.city.style.background="#ff3333";
			document.upload.city.focus();
			return false;
		} else document.upload.city.style.background=""

		if ((email == "") || (email == "undefined")) {
			alert("Email field is mandatory.");
			document.upload.email.style.background="#ff3333";
			document.upload.email.focus();
			return false;
		} else document.upload.email.style.background=""

		if (! filter.test(email)) {
			alert("Email field is not valid.");
			document.upload.email.style.background="#ff3333";
			document.upload.email.focus();
			return false;
		} else document.upload.email.style.background=""

		if ((captcha_input == "") || (captcha_input == "undefined")) {
			alert("You must retype the letters into the random image.");
			document.upload.captcha_input.style.background="#ff3333";
			document.upload.captcha_input.focus();
			return false;
		} else document.upload.captcha_input.style.background=""

		return true;
	}

	function confirmSubmit() {
		var agree=confirm("Are you sure you wish to continue?");
		if (agree)
			if(checkFields())
				return true ;
			else return false ;
		else
			return false ;
	}

	function closeThisReloadParent() {
		window.close();
		opener.location.reload();
	}

</script>

<? if($action==""): ?> 
		<form name="upload" method="post" enctype="multipart/form-data" action="<?=$_SERVER['PHP_SELF']?>?action=insert">
			<input type="hidden" name="related_to" value="<?=$relatedto?>"/>
				<div style="text-align:right">
					<h2>Upload your TBT file</h2>
					<hr/>
					<? if($relatedto != 0): ?>
					In reaction to: "<i><?=$react_title?></i>" by <?=$react_author?>
					<br/>
					<br/>
					<? endif;?>
					TBT File Title:  <input type="text" name="title" />
					<br/>
					<br/>
					Author's Name:   <input type="text" name="name" />
					<br/>
					<br/>
					Author's City:   <input type="text" name="city" />
					<br/>
					<br/>
					Author's E-mail: <input type="text" name="email" />
					<br/>
					<br/>
					<?php
						echo( captcha::form() );
					?>
					<br/>
					File: <input type="file" name="filename" />
					<hr/>
					<input type="button" onclick="window.location.reload();" name="reload" value="Reload Page" />
					<input type="submit" onclick="return confirmSubmit()" name="submit" value="Submit" />
			</div>
		</form>
<? endif; ?> 

<? 
	if($action=="insert"): 

		$title  = strip_tags(trim($_POST['title']));
		$author = strip_tags(trim($_POST['name']));
		$city = strip_tags(trim($_POST['city']));
		$email = strip_tags(trim($_POST['email']));
		$rela = strip_tags(trim($_POST['related_to']));
		$error = false;

		if(!captcha::check()) {
			$error = true;
			$msg = "Error copying the captcha letters";
		} else if(! is_uploaded_file($_FILES['filename']['tmp_name'])) {
			$error = true;
			$msg = "Error while uploading file";
		} else {
			$fname = basename($_FILES['filename']['tmp_name'])."-".time().".tbt";
			if(! move_uploaded_file($_FILES['filename']['tmp_name'], $upload_dir."/".$fname)) {
				$error = true;	
				$msg = "Error while moving file into the upload directory. Please check the permission.";
			} else {
				$TBT_DB = new TBT_DB;
				if(! $TBT_DB->insert_tbt($fname, $title, $author, $email, $city, $rela)) {
					$error = true;
					$msg = "DB: ".$TBT_DB->get_error();
				}
			}
		}
?> 
	<div style="text-align:right">
		<h2>Upload Result</h2>
	</div>

	<?=$msg?>
	<br/>
	<br/>

	<p style="text-align:center">
		<span onclick="javascript:history.back();" style="cursor:hand">Return back</span>
		<br/>
		<a href="#" onclick="closeThisReloadParent();"><u>Close this window</u></a>
	</p>
<? endif; ?> 

</body>
</html>
