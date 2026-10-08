CREATE TABLE IF NOT EXISTS `area52_archetype_vanity_delivery` (
  `guid` INT UNSIGNED NOT NULL,
  `delivered_at` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
