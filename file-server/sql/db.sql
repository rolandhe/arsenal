create database file_server;
use file_server;

DROP TABLE IF EXISTS `upload_file`;
CREATE TABLE `upload_file` (
  `id` bigint(11) NOT NULL AUTO_INCREMENT,
  `product` varchar(64) NOT NULL,
  `file_name` varchar(128) NOT NULL,
  `upload_user` varchar(64) NOT NULL,
  `name_hash` bigint NOT NULL,
  `upload_time` datetime NOT NULL,
  `proc_time` datetime NOT NULL,
  `proc_status` int NOT NULL DEFAULT 0,
  `deleted` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY name_hash_index(`name_hash`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;