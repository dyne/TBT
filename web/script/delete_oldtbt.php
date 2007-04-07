#!/usr/bin/php -q
<?

/*  Time Based Text - management script
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

# 
# WHAT IS THIS
# ============
#
# this is a management script you can use to perform some tasks:
#
# * delete the tbt file from db and filesystem older then X days
# * empty the database and the filesystem
#
# more functionalities to come in future
#
# HOW TO USE
# ==========
#
# from the cmdline: delete_oldtbt.php /path/to/tbt_webroot days
# from the crontab: scheduling execution time you want
#
# you must be sure that the user who exec this script has the permissions
# to delete .tbt file into the upload directory

function usage() {
	print("usage: \n");
	print("\n");
	print("  delete_oldtbt.php /path/to/tbt_webroot days\n");
	print("\n");
	print("    - /path/to/tbt_webroot: it's the basedir of the tbt web application site\n");
	print("    - days: the number of days. Tbts older than 'days' will be \n");
    print("            deleted from db and fs\n");
    print("            if days = all the records will be cleaned\n");
	print("\n");
	exit(1);
}

if($argc != 3) {
	usage();
}
		
$tbt_web_docroot = $argv[1];
if($argv[2] == "all") {
	$days = "all";
} else {
	$days = (int)$argv[2];
}

if(!file_exists($tbt_web_docroot)) {
	print("Error: cannot find web root: $tbt_web_docroot\n");
	exit(2);
}

if(!file_exists($tbt_web_docroot."/include/db.inc.php")) {
	print("Error: cannot find file $tbt_web_docroot"."/include/db.inc.php"."\n");
	print("Error: problably this is not a tbt site root directory...\n");
	exit(2);
}

if(!file_exists($tbt_web_docroot."/include/tbt-php.php")) {
	print("Error: cannot find file $tbt_web_docroot"."/include/tbt-php.php"."\n");
	exit(2);
}

if(!is_integer($days) && $days!="all") {
	print("Error: the days argument must be an integer!\n");
	exit(3);
}

include_once($tbt_web_docroot."/include/db.inc.php");
include_once($tbt_web_docroot."/include/tbt-php.php");

$TBT = new TBT_DB;
if($days!="all") {
	$row = $TBT->get_old_tbt($days);
} else {
	$row = $TBT->get_n("all");
}

function print_tbt($r) {
	return "id: ".$r['id']." - ".$r['title']." - by ".$r['author']." <".$r['email'].">";
}

if($row != "") {
	foreach($row as $r) {
		if($r['id']!="") {
			print("Deleting tbt id: ".$r['id']."...\n");
			if( ! $TBT->delete_tbt($r['id']) ) {
				print("Error deleting tbt: ".print_tbt($r)."\n");
			}
		}
	}
} else {
	print("No records to delete.\n");
}

exit(0);
?>
