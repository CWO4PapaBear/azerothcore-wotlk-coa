-- Shadowfang Keep: Commander Springvale casts Divine Shield once, at 33 % health, instead of every
-- 20-22 seconds.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 4278 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4278, 0, 0, 0, 0, 0, 100, 16, 4000, 8000, 15000, 21000, 0, 0, 11, 5588, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - In Combat - Cast Hammer of Justice'),
(4278, 0, 1, 0, 2, 0, 100, 16, 0, 60, 10000, 10000, 0, 0, 11, 1026, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - Between Health 0-60% - Cast Holy Light'),
(4278, 0, 2, 0, 2, 0, 100, 16, 0, 30, 30000, 30000, 0, 0, 11, 33581, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - Between Health 0-30% - Cast Divine Shield'),
(4278, 0, 3, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - On Aggro - Say Line 0'),
(4278, 0, 4, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - On Juts Died - Say Line 1'),
(4278, 0, 9000, 0, 0, 0, 100, 0, 5700, 7700, 15000, 17000, 0, 0, 11, 2102665, 2, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - Ascension kit - In Combat (first 5.7-7.7 s, then every 15-17 s) - Cast ''Consecration'''),
(4278, 0, 9001, 0, 0, 0, 100, 0, 8000, 10000, 8000, 9000, 0, 0, 218, 975011, 0, 100, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - Ascension kit - In Combat (first 8-10 s, then every 8-9 s) - Custom Cast ''Fierce Blow'' (200)'),
(4278, 0, 9002, 0, 0, 0, 100, 0, 9950, 11950, 25000, 27000, 0, 0, 11, 2102660, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - Ascension kit - In Combat (first 9.95-11.95 s, then every 25-27 s) - Cast ''Hammer of Justice'''),
(4278, 0, 9003, 0, 0, 0, 100, 0, 20000, 22000, 20000, 22000, 0, 0, 11, 2102661, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - Ascension kit - In Combat (first 20-22 s, then every 20-22 s) - Cast ''Holy Light'''),
(4278, 0, 9004, 0, 2, 0, 100, 1, 0, 33, 0, 0, 0, 0, 11, 2102659, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Commander Springvale - Ascension kit - Between 0-33% Health - Cast ''Divine Shield'' (No Repeat)');
