CREATE TABLE IF NOT EXISTS `area52_account_friend` (
  `account` int unsigned NOT NULL,
  `friend_account` int unsigned NOT NULL,
  `original_guid` int unsigned NOT NULL,
  `original_name` varchar(48) NOT NULL,
  `note` varchar(48) NOT NULL DEFAULT '',
  PRIMARY KEY (`account`, `friend_account`),
  KEY `friend_account` (`friend_account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_account_friend_migration` (
  `id` tinyint unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

START TRANSACTION;
INSERT IGNORE INTO `area52_account_friend` (`account`, `friend_account`, `original_guid`, `original_name`, `note`)
SELECT `o`.`account`, `t`.`account`, `t`.`guid`, `t`.`name`, `s`.`note`
FROM `character_social` `s`
JOIN `characters` `o` ON `o`.`guid` = `s`.`guid`
JOIN `characters` `t` ON `t`.`guid` = `s`.`friend`
WHERE (`s`.`flags` & 1) <> 0 AND `o`.`account` <> `t`.`account`
  AND `o`.`deleteinfos_name` IS NULL AND `t`.`deleteinfos_name` IS NULL
  AND NOT EXISTS (SELECT 1 FROM `area52_account_friend_migration` WHERE `id` = 1)
  AND NOT EXISTS (
    SELECT 1 FROM `character_social` `s2`
    JOIN `characters` `o2` ON `o2`.`guid` = `s2`.`guid`
    JOIN `characters` `t2` ON `t2`.`guid` = `s2`.`friend`
    WHERE (`s2`.`flags` & 1) <> 0 AND `o2`.`account` = `o`.`account` AND `t2`.`account` = `t`.`account`
      AND `o2`.`deleteinfos_name` IS NULL AND `t2`.`deleteinfos_name` IS NULL
      AND (`s2`.`guid` < `s`.`guid` OR (`s2`.`guid` = `s`.`guid` AND `s2`.`friend` < `s`.`friend`)
        OR (`s2`.`guid` = `s`.`guid` AND `s2`.`friend` = `s`.`friend` AND `s2`.`flags` < `s`.`flags`))
  );
INSERT IGNORE INTO `area52_account_friend_migration` (`id`)
SELECT 1 WHERE NOT EXISTS (SELECT 1 FROM `area52_account_friend_migration` WHERE `id` = 1);
COMMIT;
