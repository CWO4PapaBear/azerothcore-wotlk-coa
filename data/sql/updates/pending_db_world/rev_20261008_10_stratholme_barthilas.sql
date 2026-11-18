-- Stratholme: Magistrate Barthilas stands at the Service Entrance gate, on every difficulty.
--
-- His SmartAI teleported him to the inner square (4068.74, -3535.97) on every respawn and ran him
-- there on a trigger (path 104350, instance data TYPE_BARTHILAS_RUN), so after a reset or respawn
-- he was found in the wrong place. Those rows go; his fight and gate actions stay.
-- The one spawn left is the hand-placed one at the gate, and it now spawns on Normal, Heroic and
-- Mythic (it was Mythic only, so Normal and Heroic had no Barthilas at all).

DELETE FROM `creature` WHERE `id` IN (10435, 110435, 210435) AND `guid` <> 9950286;
DELETE FROM `creature` WHERE `guid` = 9950286;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) VALUES
(9950286, 10435, 329, 0, 0, 7, 1, 0, 3662.42, -3616.07, 137.481, 2.06036, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', NULL);

DELETE FROM `smart_scripts` WHERE `entryorguid` = 10435 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(10435, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 2000, 6000, 0, 0, 11, 16791, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas - In Combat - Cast ''Furious Anger'''),
(10435, 0, 1, 0, 0, 0, 100, 0, 6000, 10000, 12000, 21000, 0, 0, 11, 10887, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas - In Combat - Cast ''Crowd Pummel'''),
(10435, 0, 2, 0, 0, 0, 100, 0, 11000, 12000, 15000, 15000, 0, 0, 11, 14099, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas - In Combat - Cast ''Mighty Blow'''),
(10435, 0, 3, 0, 0, 0, 100, 0, 4000, 4000, 12000, 15000, 0, 0, 11, 16793, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas - In Combat - Cast ''Draining Blow'''),
(10435, 0, 4, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 0, 11, 16794, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas - On Just Died - Cast ''Transformation'''),
(10435, 0, 5, 0, 37, 0, 100, 0, 0, 0, 0, 0, 0, 0, 11, 16794, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas - On Initialize - Cast ''Transformation'''),
(10435, 0, 6, 7, 4, 0, 100, 512, 0, 0, 0, 0, 0, 0, 118, 1, 0, 0, 0, 0, 0, 20, 175377, 70, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas  - On aggro - Set Closest gameobject  (175377) to GO_STATE_READY'),
(10435, 0, 7, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 0, 118, 1, 0, 0, 0, 0, 0, 20, 175372, 90, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas  - On aggro - Set Closest gameobject (175372) to GO_STATE_READY'),
(10435, 0, 8, 9, 6, 0, 100, 512, 0, 0, 0, 0, 0, 0, 118, 2, 0, 0, 0, 0, 0, 20, 175377, 70, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas - On death - Set Closest gameobject (175377)  to GO_STATE_ACTIVE_ALTERNATIVE'),
(10435, 0, 9, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 0, 118, 2, 0, 0, 0, 0, 0, 20, 175372, 90, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas  - On death -  Set Closest gameobject (175372) to GO_STATE_ACTIVE_ALTERNATIVE'),
(10435, 0, 10, 11, 25, 0, 100, 512, 0, 0, 0, 0, 0, 0, 118, 2, 0, 0, 0, 0, 0, 20, 175377, 70, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas  -  On reset -  Set Closest gameobject (175377) to GO_STATE_ACTIVE_ALTERNATIVE'),
(10435, 0, 11, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 0, 118, 2, 0, 0, 0, 0, 0, 20, 175372, 90, 0, 0, 0.0, 0.0, 0.0, 0.0, 'Magistrate Barthilas - On reset - Set Closest gameobject (175372) to GO_STATE_ACTIVE_ALTERNATIVE');
