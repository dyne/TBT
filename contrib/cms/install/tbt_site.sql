DROP TABLE IF EXISTS `tbt_files`;
CREATE TABLE tbt_files (
	  `id`             int(11)      NOT NULL auto_increment,
	  `title`          varchar(255) NOT NULL default '',
	  `file`           varchar(255) NOT NULL default '',
	  `author`         varchar(255) NOT NULL default '',
	  `email`          varchar(255) default NULL,
	  `city`           varchar(255) default NULL,
	  `txt`            text         NOT NULL,
	  `reaction`_to    int(11)      default NULL,
	  `datetime`       datetime     default NULL,
	  `is_persistent`  int(11)      NOT NULL default '0',
	  PRIMARY KEY  (id),
	  KEY author (author),
	  KEY city (city)
) TYPE=MyISAM;

LOCK TABLES `tbt_files` WRITE;
UNLOCK TABLES;
