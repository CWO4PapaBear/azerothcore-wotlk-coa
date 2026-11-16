CREATE TABLE IF NOT EXISTS `area52_certification_reward_rule` (
  `account` int unsigned NOT NULL,
  `required_count` int unsigned NOT NULL DEFAULT 30,
  `item` int unsigned NOT NULL,
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `area52_certification_reward_delivery` (
  `account` int unsigned NOT NULL,
  `guid` int unsigned NOT NULL,
  `item` int unsigned NOT NULL,
  `delivered_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
