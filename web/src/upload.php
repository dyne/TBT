<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>

	<title>TBT</title>
	<style type="text/css" media="all">
		@import "styles/default.css";
	</style>
	<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">

</head>

<?
	//TODO: process the upload
?>
<body style="background-color: #990000">
<form name="upload" method="post" action="<?=$_SERVER['PHP_SELF']?>">
<p>Upload your TBT file<br />
<br />
TBT File Title:<br />
<input type="text" name="title" />
<br />
<br />
Author's Name:<br />
<input type="text" name="name" />
<br />
<br />
Author's City:<br />
<input type="text" name="city" />
<br />
<br />
Author's E-mail:<br />
<input type="text" name="email" />
<br />
<br />
File:<br />
<input type="file" name="filename" />
<br />
<br />
<input type="submit" onclick="opener.location.reload();" name="submit" value="Submit" />
</p>
</form>
</body>
</html>
