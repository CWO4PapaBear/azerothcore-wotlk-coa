-- Dire Maul West (#6851), checked against Ascension combat logs.

-- Arcane Aberration: on death it leaves Arcane Residues (2100265), a 10 s zone that restores health and mana
-- (69 deaths in the logs, never a Mana Burn).
-- Tendris Warpwood: Entangle (pulls the target in front of him, facing away), Grasping Vines and Trample
-- never come in the logs (7 pulls); Mass Entanglement, Entangling Roots and Uppercut stay.
-- Immol'thar: Ascension has no ring of Highborne Summoners, nobody fights him when the shield drops.
DELETE FROM `creature` WHERE `guid` BETWEEN 247831 AND 247848 AND `id` = 11466;
-- Eyes only from Summon Eye of Immol'thar (2 eyes), every 22 s; Eye of Immol'thar (one eye that lives 4 min),
-- Trample, Infected Bite and Portal never come in the logs.
-- Prince Tortheldrin: his Heroic melee was above his Mythic one (25.37 vs 17.83): Heroic = Mythic / 1.3.
UPDATE `creature_template` SET `DamageModifier` = 13.72 WHERE `entry` = 111486;

-- Eye of Immol'thar: on Ascension it takes 75 % more damage (888150, on it for its whole life) and casts its own
-- Eye of Immol'thar (2100245: damage over time and -20 % haste) right after it appears, not 22909. It goes for a
-- random player in range, not the nearest one.
DELETE FROM `creature_template_addon` WHERE `entry` = 14396;
INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(14396, 0, 0, 0, 0, 0, 0, '888150');
-- Dire Maul Crystal Totem (first crystal) could be attacked; on Ascension it could not, like the other crystals.
UPDATE `creature_template` SET `unit_flags` = `unit_flags` | 0x2 | 0x100 | 0x200 WHERE `entry` = 13916;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 11480 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(11480, 0, 0, 0, 0, 0, 75, 0, 1000, 4000, 6000, 11000, 0, 0, 11, 15979, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Arcane Aberration - In Combat - Cast Arcane Bolt'),
(11480, 0, 1, 0, 6, 0, 100, 528, 0, 0, 0, 0, 0, 0, 11, 22936, 258, 0, 0, 0, 0, 5, 30, 0, 1, 0, 0.0, 0.0, 0.0, 0.0, 'Arcane Aberration - On Death - Cast Mana Burn'),
(11480, 0, 9000, 0, 6, 0, 100, 512, 0, 0, 0, 0, 0, 0, 11, 2100265, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Arcane Aberration - On Death - Cast ''Arcane Residues''');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 11486 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(11486, 0, 0, 1, 60, 0, 100, 257, 5000, 5000, 5000, 5000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - On Update - Say Line 0'),
(11486, 0, 1, 0, 61, 0, 100, 512, 0, 0, 0, 0, 0, 0, 2, 14, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - On Update - Set Faction'),
(11486, 0, 2, 0, 25, 0, 100, 257, 0, 0, 0, 0, 0, 0, 11, 674, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - On Reset - Cast Dual Wield'),
(11486, 0, 3, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 0, 11, 3417, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - On Reset - Cast Thrash'),
(11486, 0, 4, 0, 0, 0, 100, 16, 3000, 5000, 10000, 20000, 0, 0, 11, 22920, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - In Combat - Cast Arcane Blast'),
(11486, 0, 5, 0, 0, 0, 100, 16, 8000, 11000, 10000, 15000, 0, 0, 11, 13736, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - In Combat - Cast Whirlwind'),
(11486, 0, 6, 0, 105, 0, 100, 16, 9000, 12000, 9000, 12000, 0, 30, 11, 20537, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Victim Casting - Cast Counterspell'),
(11486, 0, 7, 0, 6, 0, 100, 512, 0, 0, 0, 0, 0, 0, 34, 1, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - On Death - Set Instance Data 1 to 2'),
(11486, 0, 9000, 0, 0, 0, 100, 0, 8700, 10700, 9000, 10000, 0, 0, 218, 975011, 0, 100, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Ascension kit - In Combat (first 8.7-10.7 s, then every 9-10 s) - Custom Cast ''Fierce Blow'' (200)'),
(11486, 0, 9001, 0, 0, 0, 100, 0, 10000, 12000, 15000, 17000, 0, 0, 11, 2100004, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Ascension kit - In Combat (first 10-12 s, then every 15-17 s) - Cast ''Cleave'''),
(11486, 0, 9002, 0, 0, 0, 100, 0, 15000, 17000, 25000, 27000, 0, 0, 11, 2100190, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Ascension kit - In Combat (first 15-17 s, then every 25-27 s) - Cast ''Bladestorm'''),
(11486, 0, 9003, 0, 0, 0, 100, 0, 15300, 17300, 25000, 27000, 0, 0, 11, 2100247, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Ascension kit - In Combat (first 15.3-17.3 s, then every 25-27 s) - Cast ''Counterspell'''),
(11486, 0, 9004, 0, 0, 0, 100, 0, 8000, 12000, 12000, 16000, 0, 0, 11, 15589, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Ascension kit - In Combat (first 8-12 s, then every 12-16 s) - Cast ''Whirlwind'''),
(11486, 0, 9005, 0, 0, 0, 100, 0, 6000, 9000, 12000, 16000, 0, 0, 11, 22920, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Ascension kit - In Combat (first 6-9 s, then every 12-16 s) - Cast ''Arcane Blast'''),
(11486, 0, 9006, 0, 0, 0, 100, 0, 6000, 9000, 12000, 16000, 0, 0, 11, 22995, 2, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Ascension kit - In Combat (first 6-9 s, then every 12-16 s) - Cast ''Summon'''),
(11486, 0, 9007, 0, 0, 0, 100, 0, 6000, 9000, 12000, 16000, 0, 0, 11, 2100048, 2, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Ascension kit - In Combat (first 6-9 s, then every 12-16 s) - Cast ''Arcane Shock'''),
(11486, 0, 9008, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 0, 11, 2100178, 34, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Prince Tortheldrin - Ascension kit - On Aggro - Cast ''Furious Strikes''');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 11489 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(11489, 0, 0, 0, 0, 0, 100, 16, 5000, 9000, 9000, 14000, 0, 0, 11, 5568, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - In Combat - Cast Trample'),
(11489, 0, 1, 0, 0, 0, 100, 16, 2000, 4000, 17000, 22000, 0, 0, 11, 22924, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - In Combat - Cast Grasping Vines'),
(11489, 0, 2, 0, 0, 0, 100, 16, 6000, 12000, 15000, 20000, 0, 0, 11, 22994, 0, 0, 0, 0, 0, 6, 40, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - In Combat - Cast Entangle'),
(11489, 0, 3, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - On Aggro - Say Line 0'),
(11489, 0, 9000, 0, 0, 0, 100, 0, 8000, 10000, 8000, 9000, 0, 0, 218, 975011, 0, 100, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - Ascension kit - In Combat (first 8-10 s, then every 8-9 s) - Custom Cast ''Fierce Blow'' (200)'),
(11489, 0, 9001, 0, 0, 0, 100, 0, 19900, 21900, 30000, 32000, 0, 0, 11, 2100185, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - Ascension kit - In Combat (first 19.9-21.9 s, then every 30-32 s) - Cast ''Mass Entanglement'''),
(11489, 0, 9002, 0, 0, 0, 100, 0, 8350, 10350, 10000, 12000, 0, 0, 11, 2100072, 2, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - Ascension kit - In Combat (first 8.35-10.35 s, then every 10-12 s) - Cast ''Entangling Roots'''),
(11489, 0, 9003, 0, 0, 0, 100, 0, 10000, 12000, 10000, 12000, 0, 0, 11, 2100000, 2, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - Ascension kit - In Combat (first 10-12 s, then every 10-12 s) - Cast ''Uppercut'''),
(11489, 0, 9004, 0, 0, 0, 100, 16, 8000, 12000, 12000, 16000, 0, 0, 11, 5568, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - Ascension kit - In Combat (first 8-12 s, then every 12-16 s) - Cast ''Trample'''),
(11489, 0, 9005, 0, 0, 0, 100, 16, 8000, 12000, 18000, 22000, 0, 0, 11, 22924, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - Ascension kit - In Combat (first 8-12 s, then every 18-22 s) - Cast ''Grasping Vines'''),
(11489, 0, 9006, 0, 0, 0, 100, 16, 6000, 9000, 18000, 22000, 0, 0, 11, 22994, 2, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - Ascension kit - In Combat (first 6-9 s, then every 18-22 s) - Cast ''Entangle'''),
(11489, 0, 9007, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 0, 11, 2100064, 34, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Tendris Warpwood - Ascension kit - On Aggro - Cast ''Stoneform''');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 11496 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(11496, 0, 0, 1, 38, 0, 100, 512, 1, 1, 0, 0, 0, 0, 45, 2, 2, 0, 0, 0, 0, 11, 11466, 200, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - On Data Set - Set Data'),
(11496, 0, 1, 2, 61, 0, 100, 512, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 11466, 200, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - On Data Set - Say Line 0 Target'),
(11496, 0, 2, 0, 61, 0, 100, 512, 0, 0, 0, 0, 0, 0, 118, 0, 0, 0, 0, 0, 0, 20, 179503, 200, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - On Data Set - Set Gameobject State'),
(11496, 0, 3, 0, 0, 0, 100, 16, 3000, 12000, 9000, 14000, 0, 0, 11, 5568, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - In Combat - Cast Trample'),
(11496, 0, 4, 0, 0, 0, 100, 16, 3000, 9000, 10000, 15000, 0, 0, 11, 16128, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - In Combat - Cast Infected Bite'),
(11496, 0, 5, 0, 0, 0, 100, 16, 7000, 14000, 10000, 25000, 0, 0, 11, 22950, 0, 0, 0, 0, 0, 6, 50, 1, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - In Combat - Cast Portal of Immol''thar'),
(11496, 0, 6, 0, 0, 0, 100, 16, 5000, 14000, 12000, 25000, 0, 0, 11, 22899, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - In Combat - Cast Eye of Immol''thar'),
(11496, 0, 7, 0, 2, 0, 100, 16, 0, 40, 30000, 30000, 0, 0, 11, 8269, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - Between 0-40% Health - Cast Frenzy'),
(11496, 0, 8, 0, 6, 0, 100, 512, 0, 0, 0, 0, 0, 0, 34, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - On Death - Set Instance Data 1 to 1'),
(11496, 0, 9000, 0, 0, 0, 100, 0, 1000, 3000, 10000, 12000, 0, 0, 11, 2100226, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - Ascension kit - In Combat (first 1-3 s, then every 10-12 s) - Cast ''Vicious Bite'''),
(11496, 0, 9001, 0, 0, 0, 100, 0, 1000, 3000, 8000, 9000, 0, 0, 218, 975011, 0, 100, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - Ascension kit - In Combat (first 1-3 s, then every 8-9 s) - Custom Cast ''Fierce Blow'' (200)'),
(11496, 0, 9002, 0, 0, 0, 100, 0, 22000, 22000, 22000, 22000, 0, 0, 11, 2100246, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - Ascension kit - In Combat (first 1-3 s, then every 15-17 s) - Cast ''Summon Eye of Immol''thar'''),
(11496, 0, 9003, 0, 0, 0, 100, 16, 8000, 12000, 12000, 16000, 0, 0, 11, 5568, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - Ascension kit - In Combat (first 8-12 s, then every 12-16 s) - Cast ''Trample'''),
(11496, 0, 9004, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 0, 11, 8269, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - Ascension kit - Between 0-30% Health - Cast ''Frenzy'''),
(11496, 0, 9005, 0, 0, 0, 100, 16, 6000, 9000, 12000, 16000, 0, 0, 11, 16128, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - Ascension kit - In Combat (first 6-9 s, then every 12-16 s) - Cast ''Infected Bite'''),
(11496, 0, 9006, 0, 0, 0, 100, 16, 12000, 15000, 30000, 35000, 0, 0, 11, 22899, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - Ascension kit - In Combat (first 12-15 s, then every 30-35 s) - Cast ''Eye of Immol''thar'''),
(11496, 0, 9007, 0, 0, 0, 100, 16, 6000, 9000, 12000, 16000, 0, 0, 11, 22950, 2, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Immol''thar - Ascension kit - In Combat (first 6-9 s, then every 12-16 s) - Cast ''Portal of Immol''thar''');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 14396 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(14396, 0, 0, 0, 1, 0, 100, 0, 0, 0, 3000, 3000, 0, 0, 49, 0, 0, 0, 0, 0, 0, 17, 0, 60, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Eye of Immol''thar - Out of Combat - Attack Random Player'),
(14396, 0, 1, 0, 0, 0, 100, 0, 0, 500, 10000, 12000, 0, 0, 11, 2100245, 0, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Eye of Immol''thar - In Combat - Cast Eye of Immol''thar (2100245)');
