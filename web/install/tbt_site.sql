DROP TABLE IF EXISTS `tbt_files`;
CREATE TABLE `tbt_files` (
  `id`          int(11)      NOT     NULL auto_increment,
  `title`       varchar(255) NOT     NULL,
  `file`        varchar(255) NOT     NULL,
  `author`      varchar(255) NOT     NULL,
  `email`       varchar(255) default NULL,
  `city`        varchar(255) default NULL,
  `txt`         text         NOT     NULL,
  `reaction_to` int(11)      default NULL,
  `datetime`    datetime     NOT     NULL,
  PRIMARY KEY  (`id`),
  KEY          `author` (`author`),
  KEY          `city` (`city`)
) ENGINE=MyISAM;

LOCK TABLES `tbt_files` WRITE;
UNLOCK TABLES;
