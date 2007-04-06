#!/usr/bin/php -q
<?

function usage() {
	print("usage: \n");
	print("\n");
	print("  delete_oldtbt.php /path/to/tbt_webroot days\n");
	print("\n");
	print("    - /path/to/tbt_webroot: it's the basedir of the tbt web application site\n");
	print("    - days: the number of days. Tbts older than 'days' will be \n");
    print("            deleted from db and fs\n");
	print("\n");
	exit(1);
}

if($argc != 3) {
	usage();
}
		
$tbt_web_docroot = $argv[1];
$days = (int)$argv[2];

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

if(!is_integer($days)) {
	print("Error: the days argument must be an integer!\n");
	exit(3);
}

include_once($tbt_web_docroot."/include/db.inc.php");
include_once($tbt_web_docroot."/include/tbt-php.php");

$TBT = new TBT_DB;
$row = $TBT->get_old_tbt($days);

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
