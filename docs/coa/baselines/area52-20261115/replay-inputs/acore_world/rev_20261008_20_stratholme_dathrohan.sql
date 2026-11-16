-- Stratholme: Grand Crusader Dathrohan.
--
-- Melee hits half as hard on every difficulty. Fierce Blow (975011, 200 % weapon damage) follows
-- the weapon, so it halves with it (it hit for about 6k).
UPDATE `creature_template` SET `DamageModifier` = 4.3   WHERE `entry` = 10812;   -- was 8.6
UPDATE `creature_template` SET `DamageModifier` = 8.985 WHERE `entry` = 110812;  -- was 17.97
UPDATE `creature_template` SET `DamageModifier` = 20.18 WHERE `entry` = 210812;  -- was 40.36

-- Hour of Judgement at 66 % health instead of 50 %.

-- At 40 % the switch to Balnazzar starts at once. The transform cast now interrupts whatever he
-- is casting (castFlags 3 = interrupt previous + triggered; with 2 alone SmartAI waited until a
-- running Hour of Judgement had finished), and the 1.5 s until the entry changes run in phase 3,
-- where none of Dathrohan's spells fire.

DELETE FROM `smart_scripts` WHERE `entryorguid` = 10812 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(10812, 0, 0, 1, 4, 0, 100, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - On Aggro - Say Line 0'),
(10812, 0, 1, 0, 61, 0, 100, 512, 0, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - On Aggro - Set Event Phase'),
(10812, 0, 2, 0, 0, 1, 100, 0, 2000, 7000, 10000, 20000, 0, 0, 11, 17281, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - In Combat - Cast Crusader Strike'),
(10812, 0, 3, 0, 0, 1, 100, 0, 7000, 15000, 15000, 23000, 0, 0, 11, 17286, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - In Combat - Cast Crusader''s Hammer'),
(10812, 0, 4, 0, 0, 1, 100, 0, 2000, 7000, 10000, 20000, 0, 0, 11, 17284, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - In Combat - Cast Holy Strike'),
(10812, 0, 5, 6, 2, 1, 100, 513, 0, 40, 0, 0, 0, 0, 11, 17288, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - Between 0-40% Health - Cast Balnazzar Transform'),
(10812, 0, 6, 18, 61, 0, 100, 512, 0, 0, 0, 0, 0, 0, 67, 1, 1500, 1500, 0, 0, 100, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - Between 0-40% Health - Create Timed Event'),
(10812, 0, 7, 8, 59, 0, 100, 512, 1, 0, 0, 0, 0, 0, 36, 10813, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - On Timed Event - Update Entry'),
(10812, 0, 8, 0, 61, 0, 100, 512, 0, 0, 0, 0, 0, 0, 22, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - On Timed Event - Set Event Phase'),
(10812, 0, 9, 0, 0, 2, 100, 1, 1000, 1000, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - In Combat - Balnazzar Say Line 0'),
(10812, 0, 10, 0, 0, 2, 100, 0, 2000, 7000, 10000, 20000, 0, 0, 11, 17399, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - In Combat - Cast Shadow Shock'),
(10812, 0, 11, 0, 0, 2, 100, 0, 9000, 11000, 10000, 15000, 0, 0, 11, 17287, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - In Combat - Cast Mind Blast'),
(10812, 0, 12, 0, 0, 2, 100, 0, 14000, 14000, 20000, 25000, 0, 0, 11, 13704, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - In Combat - Cast Psychic Scream'),
(10812, 0, 13, 0, 0, 2, 100, 0, 11000, 11000, 20000, 25000, 0, 0, 11, 12098, 0, 0, 0, 0, 0, 6, 20, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - In Combat - Cast Sleep'),
(10812, 0, 14, 0, 0, 2, 100, 0, 20000, 20000, 20000, 25000, 0, 0, 11, 15690, 0, 0, 0, 0, 0, 6, 20, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - In Combat - Cast Mind Control'),
(10812, 0, 15, 16, 6, 0, 100, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - On Death - Say Line 1'),
(10812, 0, 16, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 0, 107, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - On Death - Summon Creature Group'),
(10812, 0, 17, 0, 1, 0, 100, 513, 0, 0, 0, 0, 0, 0, 36, 10812, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - Out of Combat - Update Entry'),
(10812, 0, 18, 0, 61, 0, 100, 512, 0, 0, 0, 0, 0, 0, 22, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Grand Crusader Dathrohan - Between 0-40% Health - Set Event Phase 3 (no spells until Balnazzar)');
