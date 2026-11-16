CREATE TABLE IF NOT EXISTS `account_freepick_mystic_collection` (
  `account` INT UNSIGNED NOT NULL,
  `spell` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`account`, `spell`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
