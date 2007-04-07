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

	$db_host="localhost";
	$db_user="tbt_site";
	$db_name="tbt_site";
	$db_pass=";.;tbt_s1t3!!!";
	$db_table="tbt_files";
	$upload_dir="upload/";

	class TBT_DB {

		var $db_handler;
		var $table;
		var $error_msg;

		// mysql> describe tbt_files;
		// +-------------+-----------+------+-----+---------+----------------+
		// | Field       | Type      | Null | Key | Default | Extra          |
		// +-------------+-----------+------+-----+---------+----------------+
		// | id          | int(11)   | NO   | PRI | NULL    | auto_increment | 
		// | file        | char(255) | NO   |     |         |                | 
		// | author      | char(255) | NO   | MUL |         |                | 
		// | email       | char(255) | YES  |     | NULL    |                | 
		// | city        | char(255) | YES  | MUL | NULL    |                | 
		// | txt         | text      | NO   |     |         |                | 
		// | reaction_to | int(11)   | YES  |     | NULL    |                | 
		// | datetime    | datetime  | YES  |     | NULL    |                | 
		// +-------------+-----------+------+-----+---------+----------------+

		// the class' constructor
		function TBT_DB() {
			global $db_host,$db_user,$db_pass,$db_name,$db_table;
			$this->connect($db_host,$db_user,$db_pass,$db_name);
			$this->table = $db_table;
		}

		// connect to the database: it will return true or
		// false depending if the connection fails
		// or not.
		function connect($host,$user,$pass,$dbname) {
			$this->db_handler = mysql_connect($host, $user, $pass);
			if(!$this->db_handler) {
				$this->error_msg="Could not connect to db: ".mysql_error();
				return false;
			}
			if(!mysql_select_db($dbname)) {
				$this->error_msg="Could not select db: ".mysql_error();
				return false;
			}
			return true;
		}

		// disconnect from the database: it will return 
		// true or false depending if the connection fails
		// or not.
		function disconnect() {
			if(mysql_close($this->db_handler)) return true;
			else return false;
		}

		// insert a tbt file into the db
		// - use the TBT_JSRender class to obtain the js string
		// - use the TBT_JSRender class to obtain the raw text
		// - insert the information into the db
		// - return the id of the created row into the db
		//
		function insert_tbt($filename,$title,$name,$email,$city,$related_to) {
			global $upload_dir;

			$TBT = new TBT_JSRender($upload_dir."/".$filename);
			if($TBT->is_error != 0) {
				$this->error_msg=$TBT->error_msg;
				return false;
			}
			if(!is_integer((int)$related_to)) {
				$this->error_msg="related_to field must be an integer number!";
				return false;
			}
			if($related_to>0) {
				if(!$this->get_by_id($related_to)) {
					$this->error_msg="There isn't any tbt file with the id: ".$related_to;
					return false;
				}
			}
			
			$filename = mysql_real_escape_string($filename);
			$title = mysql_real_escape_string($title);
			$name = mysql_real_escape_string($name);
			$email = mysql_real_escape_string($email);
			$city = mysql_real_escape_string($city);
			$txt = mysql_real_escape_string($TBT->get_text());
			$query = "INSERT INTO ".$this->table." (file,title,author,email,city,txt,reaction_to)".
				" VALUES ('$filename','$title','$name','$email','$city','".$txt."',$related_to)";

			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}

			return mysql_insert_id($this->db_handler);
		}

		// delete a tbt from the database,
		// delete also the corrispondent file
		// from the filesystem
		function delete_tbt($id) {

			$row = $this->get_by_id($id);
			if(!$row) {
				return false;
			}
			$file = $upload_dir."/".$row['file'];
			if(file_exists($file)) {
				if(unlink($file)) return true;
				else {
					$this->error_msg="Cannot delete file $file. Please check ".
						"permission and repeat";
					return false;
				}
			}

			mysql_real_escape_string($id);
			$query = "DELETE FROM ".$this->table." WHERE id=$id";
			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}
			if(mysql_affected_rows($this->db_handler)==0) {
				$this->error_msg="No entry with that id";
				return false;
			}
			return true;

		}

		// get tbts older then num days
		function get_old_tbt($days) {

			if(!is_integer($days)) {
				$this->error_msg="get_old_tbt: parameter days must be an integer!";
				return false;
			}
			$query = "SELECT * FROM ".$this->table." WHERE DATEDIFF(CURDATE(),datetime)>$days";
			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}
			if(mysql_num_rows($result)==0) {
				$this->error_msg="No entries found matching your criteria!";
				return false;
			}
			while($row[] = mysql_fetch_assoc($result));;
			mysql_free_result($result);
			return $row;

		}

		// get a multidimensional associative array with the last $num tbt's id
		// in case $num = "all" all the record will be returned
		// return false in case there aren't any record
		function get_n($num) {

			if($num == "" or $num == "all")
				$query = "SELECT * FROM ".$this->table." ORDER BY id";
			else
				$query = "SELECT * FROM ".$this->table." ORDER BY id LIMIT 0,$num";
			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}
			if(mysql_num_rows($result)==0) {
				$this->error_msg="No entries found on table!";
				return false;
			}
			while($row[] = mysql_fetch_assoc($result));;
			mysql_free_result($result);
			return $row;

		}

		// returns an associative array with these informations:
		// => author name
		// => author email
		// => author city
		// => the javascript string with the tbt data
		// => the related to field
		// => the path of the tbt file
		// => the date time of the upload
		// return false in case there isn't any 
		// record with that id
		function get_by_id($id) {

			$query = "SELECT * FROM ".$this->table." WHERE id=$id";
			mysql_real_escape_string($id);
			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}
			if(mysql_num_rows($result)==0) {
				$this->error_msg="No entry with that id";
				return false;
			}
			$row = mysql_fetch_assoc($result);
			mysql_free_result($result);
			return $row;

		}

		// returns an array that contains all
		// the ID of the tbt's that match
		// with the name parameter
		function get_all_by_name($name) {
			
			$query = "SELECT * FROM ".$this->table." WHERE name LIKE '%%s%'";
			mysql_real_escape_string($name);
			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}
			if(mysql_num_rows($result)==0) {
				$this->error_msg="No entry with that id";
				return false;
			}
			while($row[] = mysql_fetch_assoc($result));;
			mysql_free_result($result);
			return $row;

		}

		// returns an array that contains all
		// the ID of the tbt's that match
		// with the email parameter
		function get_all_by_email($email) {
			
			$query = "SELECT * FROM ".$this->table." WHERE email LIKE '%%s%'";
			mysql_real_escape_string($name);
			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}
			if(mysql_num_rows($result)==0) {
				$this->error_msg="No entry with that id";
				return false;
			}
			while($row[] = mysql_fetch_assoc($result));;
			mysql_free_result($result);
			return $row;

		}

		// returns an array that contains all
		// the ID of the tbt's that match
		// with the city parameter
		function get_all_by_city($city) {

			$query = "SELECT * FROM ".$this->table." WHERE city LIKE '%%s%'";
			mysql_real_escape_string($name);
			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}
			if(mysql_num_rows($result)==0) {
				$this->error_msg="No entry with that id";
				return false;
			}
			while($row[] = mysql_fetch_assoc($result));;
			mysql_free_result($result);
			return $row;

		}

		// returns an array that contains all
		// the ID in which there's a matching
		// into the body, the author's email
		// name and city, with the pattern
		// parameter
		//
		// the $pattern variable is a string
		// containing various words separated
		// by space
		function search($pattern, $num) {

			$array = split(" ", $pattern);
			$fields = array("title", "author", "email", "city", "txt");
			
			$query  = "SELECT * FROM ".$this->table." WHERE ";
			foreach($fields as $f) {
				foreach($array as $a) {
					$query .= "$f LIKE '%".mysql_escape_string($a)."%' OR ";
				}	
			}
			$query .= "ORDER BY id LIMIT 0,$num";
			$query  = str_replace("OR ORDER BY", "ORDER BY", $query);

			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}
			if(mysql_num_rows($result)==0) {
				$this->error_msg="No entries found on table!";
				return false;
			}

			while($row[] = mysql_fetch_assoc($result));;
			mysql_free_result($result);
			return $row;

		}

		// returns an array of tbts' id
		// related to the provided id
		function get_related($id) {

			mysql_real_escape_string($id);
			$query = "SELECT * FROM ".$this->table." WHERE reaction_to=$id";
			$result = mysql_query($query, $this->db_handler);
			if(!$result) {
				$this->error_msg="Invalid query: ".mysql_error();
				return false;
			}
			if(mysql_num_rows($result)==0) {
				$this->error_msg="No entry with that id";
				return false;
			}
			while($row[] = mysql_fetch_assoc($result));
			mysql_free_result($result);
			return $row;

		}

		function get_error()  {
			return $this->error_msg;
		}

	}
?>
