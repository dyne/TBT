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
<p>Upload your TBT<br />
<br />
Title:<br />
<input type="text" name="title" />
<br />
<br />
Name:<br />
<input type="text" name="name" />
<br />
<br />
City:<br />
<input type="text" name="city" />
<br />
<br />
E-mail:<br />
<input type="text" name="email" />
<br />
<br />
File:<br />
<input type="file" name="filename" />
<br />
<br />
Reaction to:<br />  
  <select name="select">
    <option selected="selected">no</option>
    <option>George</option>
    <option>John</option>
    <option>Paul</option>
    <option>etc.</option>
  </select>
  <br />
  <br />
  <br />
  <input type="submit" onclick="opener.location.reload();" name="Submit2" value="Submit" />
</p>
</form>
</body>
</html>
