CREATE TABLE IF NOT EXISTS `area52_certification_last_character` (
  `account` int unsigned NOT NULL,
  `guid` int unsigned NOT NULL,
  `character_name` varchar(64) NOT NULL,
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
