-- Witch Doctor Zum'rah (Zul'Farrak): Healing Wave has a 10 s cooldown and each of his heals restores
-- 7.5 % of the target's health (it healed 0.5-1 %); coa_dungeon_creature_heal is read by
-- CoADungeonSpellDamage.cpp.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 7271 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(7271, 0, 0, 0, 25, 0, 100, 512, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - On Reset - Restore faction'),
(7271, 0, 1, 0, 0, 0, 100, 16, 5000, 8000, 12000, 16000, 0, 0, 11, 15245, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - In Combat - Cast Spell Shadow Bolt Volley'),
(7271, 0, 2, 0, 0, 0, 100, 16, 1000, 1000, 3000, 4000, 0, 0, 11, 12739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - In Combat - Cast Spell Shadow Bolt'),
(7271, 0, 3, 0, 0, 0, 100, 16, 2000, 5000, 30000, 36000, 0, 0, 11, 11086, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - In Combat - Cast Spell Ward of Zumrah'),
(7271, 0, 4, 0, 2, 0, 100, 0, 0, 70, 10000, 10000, 0, 0, 11, 12491, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - Between Health 0-70% - Cast Spell Healing Wave'),
(7271, 0, 5, 0, 5, 0, 100, 0, 5000, 5000, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - On Kill - Say line 1'),
(7271, 0, 6, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - On Aggro - Say line 0'),
(7271, 0, 9000, 0, 0, 0, 100, 0, 1000, 1500, 5000, 5500, 0, 0, 11, 2102544, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - Ascension kit - In Combat (first 1-1.5 s, then every 5-5.5 s) - Cast ''Shadow Bolt'''),
(7271, 0, 9001, 0, 0, 0, 100, 0, 3100, 5100, 9000, 10000, 0, 0, 11, 380506, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - Ascension kit - In Combat (first 3.1-5.1 s, then every 9-10 s) - Cast ''Ward of Zum''rah'''),
(7271, 0, 9002, 0, 0, 0, 100, 0, 15000, 17000, 15000, 17000, 0, 0, 11, 2090114, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - Ascension kit - In Combat (first 15-17 s, then every 15-17 s) - Cast ''Shadow Bolt Volley'''),
(7271, 0, 9003, 0, 0, 0, 100, 0, 14150, 16149, 15000, 17000, 0, 0, 218, 975011, 0, 100, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Witch Doctor Zum''rah - Ascension kit - In Combat (first 14.15-16.149 s, then every 15-17 s) - Custom Cast ''Fierce Blow'' (200)');

CREATE TABLE IF NOT EXISTS `coa_dungeon_creature_heal` (
    `entry`    INT UNSIGNED NOT NULL,
    `heal_pct` FLOAT NOT NULL DEFAULT 0,
    `comment`  VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
DELETE FROM `coa_dungeon_creature_heal` WHERE `entry` = 7271;
INSERT INTO `coa_dungeon_creature_heal` (`entry`, `heal_pct`, `comment`) VALUES
(7271, 0.075, 'Witch Doctor Zum''rah: each heal restores 7.5 % of the target''s health');
