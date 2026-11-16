UPDATE `item_template` SET `ScriptName` = 'item_area52_unidentified_mystic' WHERE `entry` = 97866;

UPDATE `creature_template` SET `lootid` = 1978660000 WHERE `lootid` = 0;

DELETE FROM `creature_loot_template` WHERE `Item` = 97866;
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT DISTINCT `lootid`, 97866, 0, 25, 0, 65535, 0, 1, 1, 'Area 52 alpha: common Unidentified Mystic Scroll; mode and rate controlled by script'
FROM `creature_template` WHERE `lootid` <> 0;
