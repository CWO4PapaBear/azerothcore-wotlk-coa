-- Goldshire WB1 content, second pass: positions from Questie-X-AscensionDB v1.0.5 (Xurkon, zone 12 map
-- coordinates converted with the Elwynn WorldMapArea bounds) and the quest Seven Years of Bad Luck.
--
-- Sources:
--   quest 1660057, gameobject 2300546 and creature 162920: AscensionDB d6494ceb3150 (questcache,
--     gameobjectcache, creaturecache), matching the dawnrise questcache of 2026-09-01
--   quest givers, enders and prerequisites, townsfolk, crop, kobold warren and mirror shard positions:
--     Questie-X-AscensionDB v1.0.5
--   heights: patch-WB1.MPQ terrain and WMO collision (town hall dais, Spada manor floors, dock deck)
--
-- Not recovered: the prerequisite 1660004 of The Maid I Left Behind is absent here, so that link is left out;
-- Questie levels vary with the observer and are not applied; Questie names Cedric (162815) as the giver of
-- Agria's Medicine, which contradicts its quest text, so Dulcinea keeps it.

-- Seven Years of Bad Luck credit marker
REPLACE INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(162920, 0, 0, 0, 0, 0, '[TG] kharanos hops', '', '', 0, 1, 1, 0, 35, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 2000, 2000, 1, 1, 1, 33554432, 2048, 0, 0, 10, 1024, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.93, 1, 1, 1, 0, 0, 1, 0, 130, '', 0);
DELETE FROM `creature_template_model` WHERE `CreatureID` = 162920;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES (162920, 0, 11686, 1, 1, 0);

-- Townsfolk at their realm positions
UPDATE `creature` SET `position_x` = -9562.28, `position_y` = 63.03, `position_z` = 62.17, `orientation` = 4.71239 WHERE `guid` = 9001006;
UPDATE `creature` SET `position_x` = -9565.3, `position_y` = 9.6, `position_z` = 59.173, `orientation` = 2.23463 WHERE `guid` = 9001007;
UPDATE `creature` SET `position_x` = -9579.6, `position_y` = 35.3, `position_z` = 58.771, `orientation` = 5.04538 WHERE `guid` = 9001008;
UPDATE `creature` SET `position_x` = -9464.6, `position_y` = 39.1, `position_z` = 56.529, `orientation` = 2.08128 WHERE `guid` = 9001013;
UPDATE `creature` SET `position_x` = -9487.7, `position_y` = 36, `position_z` = 56.593, `orientation` = 5.80408 WHERE `guid` = 9001016;
UPDATE `creature` SET `position_x` = -9464.6, `position_y` = 60.2, `position_z` = 56.148, `orientation` = 4.21255 WHERE `guid` = 9001017;
UPDATE `creature` SET `position_x` = -9397.2, `position_y` = -12.3, `position_z` = 62.11, `orientation` = 2.44717 WHERE `guid` = 9001019;
UPDATE `creature` SET `position_x` = -9388.2, `position_y` = 22.1, `position_z` = 59.299, `orientation` = 2.89727 WHERE `guid` = 9001020;
UPDATE `creature` SET `position_x` = -9424.3, `position_y` = 9.24, `position_z` = 57.924, `orientation` = 2.43039 WHERE `guid` = 9001021;
UPDATE `creature` SET `position_x` = -9275.22, `position_y` = 469.02, `position_z` = 82.27, `orientation` = 4.60173 WHERE `guid` = 9001026;
UPDATE `creature` SET `position_x` = -9450.69, `position_y` = -82.02, `position_z` = 58.42, `orientation` = 0 WHERE `guid` = 9001027;
UPDATE `creature` SET `position_x` = -8920.56, `position_y` = -134.07, `position_z` = 80.68, `orientation` = 3.33358 WHERE `guid` = 9001028;

-- Seven Years of Bad Luck
REPLACE INTO `quest_template` (`ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestSortID`, `RewardXPDifficulty`, `RewardMoney`, `RewardMoneyDifficulty`, `StartItem`, `Flags`, `RewardItem1`, `RewardAmount1`, `RewardChoiceItemID1`, `RewardChoiceItemQuantity1`, `RewardChoiceItemID2`, `RewardChoiceItemQuantity2`, `RewardChoiceItemID3`, `RewardChoiceItemQuantity3`, `RewardFactionID1`, `RewardFactionValue1`, `LogTitle`, `LogDescription`, `QuestDescription`, `AreaDescription`, `QuestCompletionLog`, `RequiredNpcOrGo1`, `RequiredNpcOrGoCount1`, `ObjectiveText1`, `VerifiedBuild`) VALUES
(1660057, 2, -1, 5, 12, 5, 350, 223, 0, 8, 0, 0, 2302030, 1, 2302035, 1, 2302040, 1, 72, 5, 'Seven Years of Bad Luck', 'Inspect the broken mirror shards that Aldia Crayon blames for the curse afflicting Lady Agria Spada.', 'Hm...$b$b<The majordomo gives you a long, measuring look.>$b$bBefore you go... I need your help with one more matter.$b$bI''ve had the sense for some time that my lady''s ailments aren''t physical, but magical. A curse.$b$bIt started with a broken mirror. And you know what they say. However thorough I''ve been, shards keep turning up; bits of glass tucked around the manor and the grounds.$b$bWould you kindly deal with them?', '', 'Return to the butler.', 162920, 6, 'Mirror Shard Inspected', 0);
DELETE FROM `creature_queststarter` WHERE `quest` = 1660057;
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES (162802, 1660057);
DELETE FROM `creature_questender` WHERE `quest` = 1660057;
INSERT INTO `creature_questender` (`id`, `quest`) VALUES (162802, 1660057);

-- Quest chain
DELETE FROM `quest_template_addon` WHERE `ID` IN (1660056, 1660057, 1660059);
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(1660056, 1660055),
(1660057, 1660056),
(1660059, 1660055);

-- Mirror shards around the Spada manor
REPLACE INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `size`, `Data0`, `Data1`, `Data3`, `Data8`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(2300546, 10, 1061017, 'Mirror Shard', '', '', 1, 0, 1660057, 0, 0, 'SmartGameObjectAI', '', 0);
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2300546 AND `source_type` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2300546, 1, 0, 0, 64, 0, 100, 0, 1, 0, 0, 0, 0, 0, 33, 162920, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Mirror Shard - On Gossip Hello - Quest Credit ''Seven Years of Bad Luck'''),
(2300546, 1, 1, 0, 64, 0, 100, 0, 1, 0, 0, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mirror Shard - On Gossip Hello - Despawn In 1000 ms');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 2300546 AND `SourceId` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, 2300546, 1, 0, 9, 0, 1660057, 0, 0, 0, 0, 0, '', 'Mirror Shard - credit needs Seven Years of Bad Luck'),
(22, 2, 2300546, 1, 0, 9, 0, 1660057, 0, 0, 0, 0, 0, '', 'Mirror Shard - despawn needs Seven Years of Bad Luck');

-- Melons and pumpkins on the farm, apples in the orchard, kobold warrens south of the village and
-- mirror shards in and around the manor, replacing the estimated nodes of the first pass
DELETE FROM `gameobject` WHERE `guid` BETWEEN 9001000 AND 9001059;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`, `Comment`) VALUES
(9001000, 2300548, 0, 0, 0, 1, 1, -9505.792, 93.562, 57.036, 0, 0, 0, 0, 1, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001001, 2300548, 0, 0, 0, 1, 1, -9497.227, 92.174, 56.884, 2.39996, 0, 0, 0.9320318, 0.3623764, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001002, 2300548, 0, 0, 0, 1, 1, -9502.782, 91.827, 57.009, 4.79992, 0, 0, 0.6754927, -0.7373667, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001003, 2300548, 0, 0, 0, 1, 1, -9498.384, 95.644, 56.917, 0.91669, 0, 0, 0.4424666, 0.896785, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001004, 2300548, 0, 0, 0, 1, 1, -9501.394, 99.808, 56.958, 3.31665, 0, 0, 0.9961716, -0.0874193, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001005, 2300548, 0, 0, 0, 1, 1, -9503.246, 102.237, 56.962, 5.71661, 0, 0, 0.2795115, -0.9601423, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001006, 2300548, 0, 0, 0, 1, 1, -9499.773, 102.237, 56.949, 1.83339, 0, 0, 0.7935949, 0.6084466, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001007, 2300548, 0, 0, 0, 1, 1, -9496.532, 98.42, 56.89, 4.23335, 0, 0, 0.8546716, -0.519169, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001008, 2300548, 0, 0, 0, 1, 1, -9495.837, 101.543, 56.912, 0.35012, 0, 0, 0.1741692, 0.9847157, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001009, 2300548, 0, 0, 0, 1, 1, -9502.088, 108.483, 56.983, 2.75008, 0, 0, 0.9809012, 0.1945065, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001010, 2300547, 0, 0, 0, 1, 1, -9504.866, 75.865, 56.756, 5.15004, 0, 0, 0.5367417, -0.8437466, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001011, 2300547, 0, 0, 0, 1, 1, -9502.319, 69.966, 56.584, 1.26682, 0, 0, 0.5918962, 0.8060142, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001012, 2300547, 0, 0, 0, 1, 1, -9505.329, 68.925, 56.777, 3.66678, 0, 0, 0.9657201, -0.2595856, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001013, 2300549, 0, 0, 0, 1, 1, -9439.815, -28.929, 60.205, 6.06674, 0, 0, 0.1080121, -0.9941496, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001014, 2300549, 0, 0, 0, 1, 1, -9442.824, -29.623, 60.203, 2.18351, 0, 0, 0.887438, 0.4609271, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001015, 2300549, 0, 0, 0, 1, 1, -9443.287, -27.541, 60.154, 4.58347, 0, 0, 0.7511853, -0.6600914, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001016, 2300549, 0, 0, 0, 1, 1, -9448.843, -27.888, 60.089, 0.70025, 0, 0, 0.3430144, 0.9393302, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001017, 2300549, 0, 0, 0, 1, 1, -9447.685, -30.317, 60.163, 3.10021, 0, 0, 0.9997859, 0.0206908, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001018, 2300549, 0, 0, 0, 1, 1, -9444.213, -33.787, 60.215, 5.50017, 0, 0, 0.3815833, -0.9243345, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001019, 2300549, 0, 0, 0, 1, 1, -9442.13, -33.093, 60.214, 1.61694, 0, 0, 0.7232324, 0.6906048, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001020, 2300549, 0, 0, 0, 1, 1, -9441.666, -35.522, 60.219, 4.0169, 0, 0, 0.905748, -0.4238167, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001021, 2300549, 0, 0, 0, 1, 1, -9441.898, -42.462, 60.233, 0.13368, 0, 0, 0.066789, 0.9977671, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001022, 2300549, 0, 0, 0, 1, 1, -9444.213, -44.197, 60.226, 2.53364, 0, 0, 0.9541535, 0.2993178, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001023, 2300549, 0, 0, 0, 1, 1, -9441.898, -45.585, 60.241, 4.9336, 0, 0, 0.6247364, -0.7808357, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001024, 2300549, 0, 0, 0, 1, 1, -9445.602, -46.279, 60.213, 1.05037, 0, 0, 0.501374, 0.8652306, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001025, 2300549, 0, 0, 0, 1, 1, -9446.991, -44.197, 60.208, 3.45033, 0, 0, 0.9881086, -0.1537574, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001026, 2300549, 0, 0, 0, 1, 1, -9448.148, -45.585, 60.2, 5.85029, 0, 0, 0.2147604, -0.9766668, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001027, 2300549, 0, 0, 0, 1, 1, -9448.38, -40.033, 60.184, 1.96707, 0, 0, 0.8324604, 0.5540845, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001028, 2300549, 0, 0, 0, 1, 1, -9447.917, -38.992, 60.188, 4.36703, 0, 0, 0.8180884, -0.5750924, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001029, 2300549, 0, 0, 0, 1, 1, -9446.76, -37.604, 60.205, 0.4838, 0, 0, 0.2395485, 0.9708844, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001030, 2300579, 0, 0, 0, 1, 1, -9633.58, 133.12, 45.926, 2.88376, 0, 0, 0.9917019, 0.1285587, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001031, 2300579, 0, 0, 0, 1, 1, -9712.29, 209.46, 49.913, 5.28372, 0, 0, 0.4791902, -0.8777111, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001032, 2300579, 0, 0, 0, 1, 1, -9723.865, 171.29, 50.955, 1.4005, 0, 0, 0.6444075, 0.7646823, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001033, 2300579, 0, 0, 0, 1, 1, -9721.55, -47.32, 37.418, 3.80046, 0, 0, 0.9462263, -0.3235055, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001034, 2300579, 0, 0, 0, 1, 1, -9730.81, -12.62, 36.739, 6.20042, 0, 0, 0.0413727, -0.9991438, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001035, 2300579, 0, 0, 0, 1, 1, -9763.22, -16.09, 31.695, 2.31719, 0, 0, 0.9162413, 0.4006268, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001036, 2300579, 0, 0, 0, 1, 1, -9772.48, 32.49, 33.784, 4.71715, 0, 0, 0.7054211, -0.7087884, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001037, 2300579, 0, 0, 0, 1, 1, -9638.21, 67.19, 61.117, 0.83393, 0, 0, 0.4049854, 0.9143232, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001038, 2300579, 0, 0, 0, 1, 1, -9744.7, 67.19, 39.95, 3.23389, 0, 0, 0.9989354, -0.0461301, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001039, 2300579, 0, 0, 0, 1, 1, -9749.33, 129.65, 49.4, 5.63385, 0, 0, 0.3189959, -0.9477561, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001040, 2300579, 0, 0, 0, 1, 1, -9763.22, 67.19, 38.946, 1.75062, 0, 0, 0.7677423, 0.6407587, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001041, 2300579, 0, 0, 0, 1, 1, -9797.945, 129.65, 49.78, 4.15058, 0, 0, 0.8754192, -0.4833644, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001042, 2300579, 0, 0, 0, 1, 1, -9661.36, 105.36, 45.527, 0.26736, 0, 0, 0.1332798, 0.9910785, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001043, 2300579, 0, 0, 0, 1, 1, -9714.605, 25.55, 40.946, 2.66732, 0, 0, 0.9720141, 0.2349224, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001044, 2300579, 0, 0, 0, 1, 1, -9698.4, 164.35, 50.249, 5.06728, 0, 0, 0.5711902, -0.8208178, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001045, 2300579, 0, 0, 0, 1, 1, -9647.47, 202.52, 49.271, 1.18405, 0, 0, 0.5580424, 0.8298124, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001046, 2300579, 0, 0, 0, 1, 1, -9714.605, 112.3, 46.251, 3.58401, 0, 0, 0.975633, -0.2194089, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001047, 2300579, 0, 0, 0, 1, 1, -9698.4, 63.72, 56.651, 5.98397, 0, 0, 0.1490503, -0.9888296, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001048, 2300546, 0, 0, 0, 1, 1, -9299.294, 462.076, 86.04, 2.10074, 0, 0, 0.8676084, 0.4972481, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001049, 2300546, 0, 0, 0, 1, 1, -9283.783, 466.24, 82.266, 4.5007, 0, 0, 0.7778519, -0.6284476, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001050, 2300546, 0, 0, 0, 1, 1, -9276.144, 456.524, 82.266, 0.61748, 0, 0, 0.3038581, 0.9527173, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001051, 2300546, 0, 0, 0, 1, 1, -9270.588, 457.565, 82.266, 3.01744, 0, 0, 0.9980739, 0.0620369, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001052, 2300546, 0, 0, 0, 1, 1, -9302.072, 443.685, 78.329, 5.4174, 0, 0, 0.4194988, -0.9077559, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001053, 2300546, 0, 0, 0, 1, 1, -9304.156, 451.666, 78.577, 1.53417, 0, 0, 0.694041, 0.7199355, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001054, 2300546, 0, 0, 0, 1, 1, -9309.017, 429.111, 77.453, 3.93413, 0, 0, 0.9225069, -0.3859806, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001055, 2300546, 0, 0, 0, 1, 1, -9314.805, 481.855, 78.049, 0.05091, 0, 0, 0.0254515, 0.9996761, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001056, 2300546, 0, 0, 0, 1, 1, -9310.406, 501.634, 77.434, 2.45087, 0, 0, 0.9409529, 0.3385374, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001057, 2300546, 0, 0, 0, 1, 1, -9284.246, 478.732, 77.812, 4.85083, 0, 0, 0.6565067, -0.7543202, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001058, 2300546, 0, 0, 0, 1, 1, -9281.005, 491.918, 77.798, 0.9676, 0, 0, 0.4651478, 0.885233, 120, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001059, 2300546, 0, 0, 0, 1, 1, -9270.125, 452.36, 79.237, 3.36756, 0, 0, 0.9936239, -0.1127451, 120, 100, 1, '', 0, 'Goldshire WB1 content');
