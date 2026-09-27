-- Goldshire content from the Conquest of Azeroth realm: townsfolk, market vendors, the refugee camp,
-- harvest nodes, kobold warrens and the quests The Maid I Left Behind, Agria's Medicine, Worm-Eaten Apple,
-- Goldshire's Generosity and Stay a While.
--
-- Sources:
--   names, subnames, health modifiers, quest records, gameobject records: AscensionDB d6494ceb3150
--     (creaturecache, questcache, gameobjectcache)
--   which market vendor sells which ingredient: db.exil.es only
--   quest enders: the questcache completion texts; quest givers: the realm footage
--   positions: patch-WB1.MPQ geometry and the realm footage, estimated to the nearest free spot
--
-- Not recovered: the realm models 652000-652448 are absent from the server CreatureDisplayInfo.dbc, so
-- stock human displays stand in; levels, faction and flags follow Remy "Two Times" (241), Garion Hunter
-- follows Randal Hunter (4732), the objective credit markers 162921/162940 follow Invisible Stalker
-- (15214); Aliscar's gossip option text, harvest yields and respawn times are chosen here.

-- Creature templates
REPLACE INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(161700, 0, 0, 0, 0, 0, 'Bianca Spada', NULL, NULL, 0, 5, 5, 0, 12, 2, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162800, 0, 0, 0, 0, 0, 'Dulcinea', 'Maid of House Spada', NULL, 0, 5, 5, 0, 12, 2, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1.0, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162801, 0, 0, 0, 0, 0, 'Eldor Hammer', 'Westfall Refugee', NULL, 0, 5, 5, 0, 12, 2, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1.0, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162802, 0, 0, 0, 0, 0, 'Aldia Crayon', 'Majordomo', NULL, 0, 5, 5, 0, 12, 2, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1.0, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162805, 0, 0, 0, 0, 0, 'Clara the Mad', NULL, NULL, 0, 5, 5, 0, 12, 2, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162806, 0, 0, 0, 0, 0, 'Aliscar Lend', NULL, NULL, 1628000, 5, 5, 0, 12, 3, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'SmartAI', 0, 1, 1.0, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162807, 0, 0, 0, 0, 0, 'Harvend Thorm', 'Mayor of Goldshire', NULL, 0, 5, 5, 0, 12, 2, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1.0, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162809, 0, 0, 0, 0, 0, 'Rowena', NULL, NULL, 0, 5, 5, 0, 12, 128, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162810, 0, 0, 0, 0, 0, 'Isolde', 'Maid of House Bruck', NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.98, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162811, 0, 0, 0, 0, 0, 'Darron', NULL, NULL, 0, 5, 5, 0, 12, 128, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162812, 0, 0, 0, 0, 0, 'Cerys', 'Maid of House Mortel', NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162813, 0, 0, 0, 0, 0, 'Alandra', NULL, NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162814, 0, 0, 0, 0, 0, 'Ainora', 'Florist', NULL, 0, 5, 5, 0, 12, 128, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162817, 0, 0, 0, 0, 0, 'Westfall Refugee', NULL, NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162818, 0, 0, 0, 0, 0, 'Westfall Refugee', NULL, NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162819, 0, 0, 0, 0, 0, 'Westfall Refugee', NULL, NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162820, 0, 0, 0, 0, 0, 'Westfall Refugee', NULL, NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162821, 0, 0, 0, 0, 0, 'Goldshire Farmer', NULL, NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.93, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162822, 0, 0, 0, 0, 0, 'Goldshire Farmer', NULL, NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162823, 0, 0, 0, 0, 0, 'Goldshire Farmer', NULL, NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162824, 0, 0, 0, 0, 0, 'Goldshire Farmer', NULL, NULL, 0, 5, 5, 0, 12, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162826, 0, 0, 0, 0, 0, 'Joaquin', NULL, NULL, 0, 5, 5, 0, 12, 128, 1, 1.14286, 1, 1, 18, 0, 0, 1, 1500, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.96, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(97921, 0, 0, 0, 0, 0, 'Garion Hunter', 'Traveling Riding Trainer', NULL, 4018, 10, 10, 0, 12, 83, 1, 1.14286, 1, 1, 18, 0, 0, 1, 2000, 2000, 1, 1, 1, 512, 2048, 0, 0, 7, 134217728, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1.225, 1, 1, 1, 0, 0, 1, 0, 2, '', 0),
(162921, 0, 0, 0, 0, 0, 'Listen to Aliscar Lend', '', '', 0, 1, 1, 0, 35, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 2000, 2000, 1, 1, 1, 33554432, 2048, 0, 0, 10, 1024, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.93, 1, 1, 1, 0, 0, 1, 0, 130, '', 0),
(162940, 0, 0, 0, 0, 0, 'Kobold Warren Destroyed', '', '', 0, 1, 1, 0, 35, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 2000, 2000, 1, 1, 1, 33554432, 2048, 0, 0, 10, 1024, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.93, 1, 1, 1, 0, 0, 1, 0, 130, '', 0);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (161700, 162800, 162801, 162802, 162805, 162806, 162807, 162809, 162810, 162811, 162812, 162813, 162814, 162817, 162818, 162819, 162820, 162821, 162822, 162823, 162824, 162826, 97921, 162921, 162940);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(161700, 0, 25562, 1, 1, 0),
(162800, 0, 25554, 1, 1, 0),
(162801, 0, 3553, 1, 1, 0),
(162802, 0, 25557, 1, 1, 0),
(162805, 0, 18618, 1, 1, 0),
(162806, 0, 15767, 1, 1, 0),
(162807, 0, 15766, 1, 1, 0),
(162809, 0, 15768, 1, 1, 0),
(162810, 0, 25555, 1, 1, 0),
(162811, 0, 3642, 1, 1, 0),
(162812, 0, 25556, 1, 1, 0),
(162813, 0, 15769, 1, 1, 0),
(162814, 0, 19177, 1, 1, 0),
(162817, 0, 3554, 1, 1, 0),
(162818, 0, 18026, 1, 1, 0),
(162819, 0, 18619, 1, 1, 0),
(162820, 0, 25564, 1, 1, 0),
(162821, 0, 3614, 1, 1, 0),
(162822, 0, 16920, 1, 1, 0),
(162823, 0, 19178, 1, 1, 0),
(162824, 0, 25563, 1, 1, 0),
(162826, 0, 11354, 1, 1, 0),
(97921, 0, 3274, 1, 1, 0),
(162921, 0, 11686, 1, 1, 0),
(162940, 0, 11686, 1, 1, 0);

-- Garion Hunter teaches riding like Randal Hunter
DELETE FROM `creature_default_trainer` WHERE `CreatureId` = 97921;
INSERT INTO `creature_default_trainer` (`CreatureId`, `TrainerId`) VALUES (97921, 37);

-- Market vendors for the potion ingredients
DELETE FROM `npc_vendor` WHERE `entry` IN (162809, 162811, 162814, 162826);
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild`) VALUES
(162809, 0, 558958, 0, 0, 0, 0),
(162811, 0, 558957, 0, 0, 0, 0),
(162814, 0, 558956, 0, 0, 0, 0),
(162826, 0, 558959, 0, 0, 0, 0);

-- Spawns: townsfolk in the market, town hall and refugee camp, farmers in the fields, Joaquin on the
-- dock, Garion Hunter by the stable and Bianca Spada at Northshire Abbey
DELETE FROM `creature` WHERE `guid` BETWEEN 9001006 AND 9001028;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`) VALUES
(9001006, 162807, 0, 0, 0, 1, 1, 0, -9556, 49.5, 60.814, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001007, 162801, 0, 0, 0, 1, 1, 0, -9556.866, 19.5, 58.583, 3.00459, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001008, 162805, 0, 0, 0, 1, 1, 0, -9585.5, 30, 59.2, 5.63211, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001009, 162817, 0, 0, 0, 1, 1, 0, -9566, 12, 59.09, 2.30361, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001010, 162818, 0, 0, 0, 1, 1, 0, -9575, 18, 59.465, 1.5708, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001011, 162819, 0, 0, 0, 1, 1, 0, -9582, 34, 58.933, 5.24046, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001012, 162820, 0, 0, 0, 1, 1, 0, -9573.374, 32.668, 58.432, 4.56116, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001013, 162800, 0, 0, 0, 1, 1, 0, -9467.5, 44, 56.541, 5.27099, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001014, 162810, 0, 0, 0, 1, 1, 0, -9468.5, 39.5, 56.538, 0.69866, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001015, 162812, 0, 0, 0, 1, 1, 0, -9464.5, 44.5, 56.527, 4.23504, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001016, 162809, 0, 0, 0, 1, 1, 0, -9489.5, 36.5, 56.595, 5.84081, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001017, 162811, 0, 0, 0, 1, 1, 0, -9478.25, 30.433, 56.679, 2.41131, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001018, 162813, 0, 0, 0, 1, 1, 0, -9498.5, 20, 56.708, 0.57542, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001019, 162806, 0, 0, 0, 1, 1, 0, -9456, 27, 56.689, 2.9362, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001020, 162814, 0, 0, 0, 1, 1, 0, -9386, 23.5, 59.514, 3.06437, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001021, 97921, 0, 0, 0, 1, 1, 0, -9436, -14, 58.869, 4.71239, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001022, 162821, 0, 0, 0, 1, 1, 0, -9500, 95, 56.973, 5.1448, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001023, 162822, 0, 0, 0, 1, 1, 0, -9398, -50, 64.471, 2.30361, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001024, 162823, 0, 0, 0, 1, 1, 0, -9445, -40, 60.225, 1.91382, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001025, 162824, 0, 0, 0, 1, 1, 0, -9508, 80, 56.959, 5.36226, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001026, 162802, 0, 0, 0, 1, 1, 0, -9411.964, 70.322, 58.159, 3.7488, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001027, 162826, 0, 0, 0, 1, 1, 0, -9456.5, -84, 57.338, 1.5708, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001028, 161700, 0, 0, 0, 1, 1, 0, -8945.5, -135, 83.74, 3.33358, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content');

-- Quests
REPLACE INTO `quest_template` (`ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestSortID`, `RewardXPDifficulty`, `RewardMoney`, `RewardMoneyDifficulty`, `StartItem`, `Flags`, `RewardItem1`, `RewardAmount1`, `RewardChoiceItemID1`, `RewardChoiceItemQuantity1`, `RewardChoiceItemID2`, `RewardChoiceItemQuantity2`, `RewardChoiceItemID3`, `RewardChoiceItemQuantity3`, `RewardFactionID1`, `RewardFactionValue1`, `LogTitle`, `LogDescription`, `QuestDescription`, `AreaDescription`, `QuestCompletionLog`, `RequiredNpcOrGo1`, `RequiredNpcOrGoCount1`, `RequiredItemId1`, `RequiredItemId2`, `RequiredItemId3`, `RequiredItemId4`, `RequiredItemId5`, `RequiredItemCount1`, `RequiredItemCount2`, `RequiredItemCount3`, `RequiredItemCount4`, `RequiredItemCount5`, `ObjectiveText1`, `VerifiedBuild`) VALUES
(1660055, 2, -1, 5, 12, 4, 120, 78, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 72, 5, 'The Maid I Left Behind', 'Find Dulcinea, Bianca''s maid, in Goldshire.', 'You''ve been in and out of the abbey all day, yet I still haven''t seen my brother anywhere.$b$bI take it you couldn''t snap him out of it. Well... worth a try.$b$b<Her long, weary sigh says enough.>$b$bHeading out again? Maybe you can do me one more favor. On the way to the abbey I left one of my maids in Goldshire with a long list of ingredients to buy; medicine for my mother.$b$bHer name is Dulcinea. Could you find her and let her know I''ll be late?', '', 'Speak with Dulcinea.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(1660056, 2, -1, 5, 12, 5, 110, 70, 558960, 8, 0, 0, 2302015, 1, 2302020, 1, 0, 0, 72, 5, 'Agria''s Medicine', 'Buy the potion''s ingredients at the Goldshire market: Elgris Blossom Petals, Dun Kazad Liquor Concentrate, Pumpkin Juice, and a Murloc Eyeball.', 'Lady Agria Spada is a woman besieged by age. Of late she''s borne a litany of ailments that keep her to her bed.$b$bOnly one thing eases her: the draught brewed by her master alchemist.$b$bI''ve gathered a few ingredients, but the list is long and fussy. We still need Elgris Blossom Petals, Dun Kazad Liquor Concentrate, Pumpkin Juice, and a Murloc Eyeball.$b$bTake a turn through the market. Once you''ve got the lot, report to my lady''s manor. I don''t dare the roads alone, but you... you''re made of sterner stuff, aren''t you?', '', 'Speak with Aldia Crayon, butler to the Spadas, at the family manor.', 0, 0, 558956, 558957, 558958, 558959, 558960, 1, 1, 1, 1, 1, '', 0),
(1660058, 2, -1, 5, 12, 5, 260, 196, 0, 8, 2302045, 1, 0, 0, 0, 0, 0, 0, 72, 5, 'Worm-Eaten Apple', 'Find and destroy the kobold warrens around Goldshire.', 'Whispers. They think no one''s noticed,but I sleep with my ear to the floor! Ha!$b$bThey dig and dig and dig. They''ll strike when you least expect it... unless you do something first.$b$bKobolds and kobolds and more kobolds. Under our feet! Watch where you step. Mind your footing!$b$bFind their warrens and set them alight. Crush them. With a big, heavy hammer!$b$b<She laughs to herself, then stares into the middle distance.>', '', 'Return to Clara the Mad.', 162940, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Kobold Warren Destroyed', 0),
(1660059, 2, -1, 5, 12, 5, 156, 124, 0, 8, 0, 0, 2302050, 1, 2302055, 1, 2302060, 1, 72, 5, 'Goldshire''s Generosity', 'Gather Melons, Pumpkins, and Apples from the farms around Goldshire. Then deliver them to Eldor Hammer, leader of the refugees.', 'Welcome.$b$bI am Thorm—Harvend Thorm—mayor of Goldshire, serving on behalf of Lord Bruk and his house.$b$bYou may have noticed the makeshift camp in the shadow of this grand hall. Of late, Goldshire has taken in countless refugees out of Westfall. At times I fear it''s beyond our means...$b$bEven so, I won''t turn away while they go hungry. Walk the village fields and take a little from each harvest to bring to the camp. You have my leave—and, by extension, Lord Bruk''s.', '', 'Deliver the basket of food to Eldor Hammer.', 0, 0, 558961, 558962, 558963, 0, 0, 3, 3, 10, 0, 0, '', 0),
(1660060, 2, -1, 5, 12, 3, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 72, 5, 'Stay a While', 'Take a moment from the noise and haste and stay a while to listen to Aliscar Lend.', 'As it turns out, I''m the leading authority around here when it comes to the village''s history. Care for a short lesson?$b$b<The aging sorcerer seems eager, almost desperate, to talk.>$b$b<Perhaps he has something worth sharing.>', '', 'Say goodbye to Aliscar Lend.', 162921, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Listen to Aliscar Lend', 0);

DELETE FROM `creature_queststarter` WHERE `quest` IN (1660055, 1660056, 1660058, 1660059, 1660060);
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES
(161700, 1660055),
(162800, 1660056),
(162805, 1660058),
(162807, 1660059),
(162806, 1660060);
DELETE FROM `creature_questender` WHERE `quest` IN (1660055, 1660056, 1660058, 1660059, 1660060);
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(162800, 1660055),
(162802, 1660056),
(162805, 1660058),
(162801, 1660059),
(162806, 1660060);

-- Aliscar Lend's history lesson
DELETE FROM `npc_text` WHERE `ID` = 1628000;
INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `VerifiedBuild`) VALUES
(1628000, 'As it turns out, I''m the leading authority around here when it comes to the village''s history. Care for a short lesson?$b$b<The aging sorcerer seems eager, almost desperate, to talk.>$b$b<Perhaps he has something worth sharing.>', 'As it turns out, I''m the leading authority around here when it comes to the village''s history. Care for a short lesson?$b$b<The aging sorcerer seems eager, almost desperate, to talk.>$b$b<Perhaps he has something worth sharing.>', 0, 0, 1, 0);
DELETE FROM `gossip_menu` WHERE `MenuID` = 1628000;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`) VALUES (1628000, 1628000);
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 1628000;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(1628000, 0, 0, 'Tell me about the history of Goldshire.', 0, 1, 1, 0, 0, 0, 0, '', 0, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 15 AND `SourceGroup` = 1628000;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(15, 1628000, 0, 0, 0, 9, 0, 1660060, 0, 0, 0, 0, 0, '', 'Aliscar Lend - show lesson while Stay a While is taken');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 162806 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(162806, 0, 0, 0, 62, 0, 100, 0, 1628000, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Aliscar Lend - On Gossip Option 0 Selected - Close Gossip'),
(162806, 0, 1, 0, 62, 0, 100, 0, 1628000, 0, 0, 0, 0, 0, 33, 162921, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Aliscar Lend - On Gossip Option 0 Selected - Quest Credit ''Stay a While''');

-- Harvest nodes for Goldshire's Generosity and the kobold warrens of Worm-Eaten Apple
REPLACE INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `size`, `Data0`, `Data1`, `Data3`, `Data8`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(2300547, 3, 60, 'Pumpkin', '', '', 0.5, 1689, 2300547, 1, 1660059, '', '', 0),
(2300548, 3, 332, 'Melon', '', '', 0.7, 1689, 2300548, 1, 1660059, '', '', 0),
(2300549, 3, 433, 'Apple', '', '', 0.5, 1689, 2300549, 1, 1660059, '', '', 0),
(2300579, 10, 1017889, 'Kobold Warren', '', '', 0.3, 0, 1660058, 0, 0, 'SmartGameObjectAI', '', 0);
DELETE FROM `gameobject_loot_template` WHERE `Entry` IN (2300547, 2300548, 2300549);
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(2300547, 558961, 0, 100, 1, 1, 0, 1, 1, 'Pumpkin'),
(2300548, 558962, 0, 100, 1, 1, 0, 1, 1, 'Melon'),
(2300549, 558963, 0, 100, 1, 1, 0, 1, 2, 'Apple');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2300579 AND `source_type` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2300579, 1, 0, 0, 64, 0, 100, 0, 1, 0, 0, 0, 0, 0, 33, 162940, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Kobold Warren - On Gossip Hello - Quest Credit ''Worm-Eaten Apple'''),
(2300579, 1, 1, 0, 64, 0, 100, 0, 1, 0, 0, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kobold Warren - On Gossip Hello - Despawn In 1000 ms');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 2300579 AND `SourceId` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, 2300579, 1, 0, 9, 0, 1660058, 0, 0, 0, 0, 0, '', 'Kobold Warren - credit needs Worm-Eaten Apple'),
(22, 2, 2300579, 1, 0, 9, 0, 1660058, 0, 0, 0, 0, 0, '', 'Kobold Warren - despawn needs Worm-Eaten Apple');

DELETE FROM `gameobject` WHERE `guid` BETWEEN 9001000 AND 9001031;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`, `Comment`) VALUES
(9001000, 2300547, 0, 0, 0, 1, 1, -9407.587, -63.364, 64.452, 0, 0, 0, 0, 1, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001001, 2300547, 0, 0, 0, 1, 1, -9411.624, -55.644, 64.445, 2.39996, 0, 0, 0.9320318, 0.3623764, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001002, 2300547, 0, 0, 0, 1, 1, -9407.899, -53.462, 64.469, 4.79992, 0, 0, 0.6754927, -0.7373667, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001003, 2300547, 0, 0, 0, 1, 1, -9418.772, -51.89, 64.37, 0.91669, 0, 0, 0.4424666, 0.896785, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001004, 2300547, 0, 0, 0, 1, 1, -9413.452, -46.728, 64.507, 3.31665, 0, 0, 0.9961716, -0.0874193, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001005, 2300547, 0, 0, 0, 1, 1, -9402.089, -38.794, 64.924, 5.71661, 0, 0, 0.2795115, -0.9601423, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001006, 2300548, 0, 0, 0, 1, 1, -9463.851, -57.21, 57.225, 1.83339, 0, 0, 0.7935949, 0.6084466, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001007, 2300548, 0, 0, 0, 1, 1, -9476.335, -57.085, 57.29, 4.23335, 0, 0, 0.8546716, -0.519169, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001008, 2300548, 0, 0, 0, 1, 1, -9484.978, -47.204, 57.174, 0.35012, 0, 0, 0.1741692, 0.9847157, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001009, 2300548, 0, 0, 0, 1, 1, -9481.031, -56.331, 57.335, 2.75008, 0, 0, 0.9809012, 0.1945065, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001010, 2300548, 0, 0, 0, 1, 1, -9482.723, -48.462, 57.217, 5.15004, 0, 0, 0.5367417, -0.8437466, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001011, 2300548, 0, 0, 0, 1, 1, -9461.378, -54.306, 57.245, 1.26682, 0, 0, 0.5918962, 0.8060142, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001012, 2300548, 0, 0, 0, 1, 1, -9497.936, 97.236, 56.91, 3.66678, 0, 0, 0.9657201, -0.2595856, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001013, 2300549, 0, 0, 0, 1, 1, -9441.496, -34.62, 60.218, 6.06674, 0, 0, 0.1080121, -0.9941496, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001014, 2300549, 0, 0, 0, 1, 1, -9442.781, -37.785, 60.222, 2.18351, 0, 0, 0.887438, 0.4609271, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001015, 2300549, 0, 0, 0, 1, 1, -9445.893, -37.393, 60.221, 4.58347, 0, 0, 0.7511853, -0.6600914, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001016, 2300549, 0, 0, 0, 1, 1, -9448.738, -40.203, 60.179, 0.70025, 0, 0, 0.3430144, 0.9393302, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001017, 2300549, 0, 0, 0, 1, 1, -9445.659, -45.317, 60.217, 3.10021, 0, 0, 0.9997859, 0.0206908, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001018, 2300549, 0, 0, 0, 1, 1, -9448.504, -48.127, 60.156, 5.50017, 0, 0, 0.3815833, -0.9243345, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001019, 2300549, 0, 0, 0, 1, 1, -9440.165, -43.237, 60.241, 1.61694, 0, 0, 0.7232324, 0.6906048, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001020, 2300549, 0, 0, 0, 1, 1, -9443.01, -46.047, 60.231, 4.0169, 0, 0, 0.905748, -0.4238167, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001021, 2300549, 0, 0, 0, 1, 1, -9446.256, -28.41, 60.157, 0.13368, 0, 0, 0.066789, 0.9977671, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001022, 2300549, 0, 0, 0, 1, 1, -9449.102, -31.221, 60.135, 2.53364, 0, 0, 0.9541535, 0.2993178, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001023, 2300549, 0, 0, 0, 1, 1, -9440.843, -26.199, 60.139, 4.9336, 0, 0, 0.6247364, -0.7808357, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001024, 2300549, 0, 0, 0, 1, 1, -9443.102, -29.553, 60.202, 1.05037, 0, 0, 0.501374, 0.8652306, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001025, 2300579, 0, 0, 0, 1, 1, -9402.4, -57.4, 64.454, 3.45033, 0, 0, 0.9881086, -0.1537574, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001026, 2300579, 0, 0, 0, 1, 1, -9445.077, -47.78, 60.207, 5.85029, 0, 0, 0.2147604, -0.9766668, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001027, 2300579, 0, 0, 0, 1, 1, -9521.555, -23.96, 56.483, 1.96707, 0, 0, 0.8324604, 0.5540845, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001028, 2300579, 0, 0, 0, 1, 1, -9379.4, 40.1, 59.664, 4.36703, 0, 0, 0.8180884, -0.5750924, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001029, 2300579, 0, 0, 0, 1, 1, -9570.736, 8.536, 59.399, 0.4838, 0, 0, 0.2395485, 0.9708844, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001030, 2300579, 0, 0, 0, 1, 1, -9576.977, 26.72, 58.844, 2.88376, 0, 0, 0.9917019, 0.1285587, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001031, 2300579, 0, 0, 0, 1, 1, -9550.632, 16.96, 58.698, 5.28372, 0, 0, 0.4791902, -0.8777111, 120, 100, 1, '', 0, 'Goldshire WB1 content');
