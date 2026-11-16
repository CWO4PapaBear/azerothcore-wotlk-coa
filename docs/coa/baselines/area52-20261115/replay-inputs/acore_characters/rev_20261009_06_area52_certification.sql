CREATE TABLE IF NOT EXISTS `area52_tester_certification` (
  `entry` int unsigned NOT NULL,
  `account` int unsigned NOT NULL,
  `guid` int unsigned NOT NULL,
  `character_name` varchar(64) NOT NULL,
  `checks_confirmed` tinyint unsigned NOT NULL DEFAULT 7,
  `evidence_sha256` varchar(64) NOT NULL,
  `certified_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
