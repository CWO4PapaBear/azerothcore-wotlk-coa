INSERT IGNORE INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `Data1`) VALUES
(48, 4, 7391, 'Felforged Enchanting Altar', 0.5, 45004),
(80148, 4, 8304, 'Ancient Enchanting Altar', 2.0, 45004),
(245006, 4, 1056774, 'Mechanical Mystic Altar', 0.5, 45004),
(357264, 4, 124539, 'Scribe''s Table', 0.75, 45004),
(800079, 4, 1080800, 'Portable Mystic Altar (League 4)', 0.4, 45004),
(1000079, 4, 1006529, 'Altar of the Tirisgarde', 0.9, 45004),
(3241144, 4, 1028888, 'Altar of the Silver Hand', 0.6, 45004),
(3244734, 4, 1029774, 'Altar of the Dreamweavers', 0.7, 45004),
(3245004, 4, 1031145, 'Altar of the Black Harvest', 1.0, 45004),
(3245006, 4, 1031585, 'Forge of the Guardian', 0.8, 45004),
(3245037, 4, 1031701, 'Altar of the Unseen Path', 0.4, 45004),
(3245726, 4, 1031101, 'Altar of the Valarjar', 0.55, 45004),
(3246320, 4, 1030399, 'Altar of the Conclave', 0.5, 45004),
(3249516, 4, 1033322, 'Altar of the Uncrowned', 0.5, 45004),
(3252409, 4, 1006528, 'Altar of the Maelstrom', 0.3, 45004);

DELETE FROM `spell_script_names` WHERE `spell_id` = 91601 AND `ScriptName` = 'spell_freepick_ancient_altar_scroll';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(91601, 'spell_freepick_ancient_altar_scroll');

UPDATE `item_template` SET `spellid_1` = 985212 WHERE `entry` = 1903514 AND `spellid_1` = 985211;
