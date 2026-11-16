-- Adapted from upstream PR #7126 to the existing Area 52 Landslide AI block.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 12203 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(12203, 0, 0, 0, 0, 0, 100, 0, 12000, 17000, 45000, 45000, 0, 0, 11, 21808, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Landslide - In Combat - Cast Landslide'),
(12203, 0, 1, 0, 0, 0, 100, 0, 3000, 12000, 12000, 15000, 0, 0, 11, 10101, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Landslide - In Combat - Cast Knock Away'),
(12203, 0, 2, 0, 0, 0, 100, 0, 3000, 7000, 8000, 15000, 0, 0, 11, 5568, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Landslide - In Combat - Cast Trample'),
(12203, 0, 3, 0, 6, 0, 100, 512, 0, 0, 0, 0, 0, 0, 41, 500, 0, 0, 0, 0, 0, 11, 11783, 100, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Landslide - On death - Despawn minions');
