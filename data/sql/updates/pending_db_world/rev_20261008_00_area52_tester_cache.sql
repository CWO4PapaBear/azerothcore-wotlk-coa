INSERT INTO `item_template` (`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`, `Flags`,
 `BuyCount`, `BuyPrice`, `SellPrice`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`,
 `maxcount`, `stackable`, `bonding`, `description`, `spellid_1`, `spelltrigger_1`, `ScriptName`)
VALUES (9900052, 15, 0, 'Area 52 Experimental Supply Crate', 900052, 3, 0,
 1, 0, 0, -1, -1, 1, 0, 1, 1, 1,
 'Unstable ideas. Unlimited possibilities. Everything an Area 52 tester needs to build something dangerous.',
 93461, 0, 'item_area52_tester_cache')
ON DUPLICATE KEY UPDATE `name` = VALUES(`name`), `displayid` = VALUES(`displayid`),
 `Flags` = VALUES(`Flags`), `spellid_1` = VALUES(`spellid_1`), `spelltrigger_1` = VALUES(`spelltrigger_1`),
 `ScriptName` = VALUES(`ScriptName`), `description` = VALUES(`description`);

UPDATE `item_template` SET `stackable` = 500 WHERE `entry` = 97866;

UPDATE `item_template` SET `BuyPrice` = 5000 WHERE `entry` = 3818046;
