-- Goldshire WB1 content, third pass: the Defias chain of Melika Isenstrider, Remy "Two Times" and Marshal
-- Dughan at the Bandit Bastion, east of Goldshire.
--
-- Sources:
--   quests 100071, 100073 and 100074, items 5055564 and 5055565 and creature 991516: AscensionDB
--     d6494ceb3150 (questcache, itemcache, creaturecache), matching the dawnrise questcache of 2026-09-01
--   quest givers, enders and the Gold Dust Exchange prerequisite, the Suspicious Guard drop and the crate
--     and guard positions: Questie-X-AscensionDB v1.0.5
--   heights: patch-WB1.MPQ terrain and WMO collision
--
-- Estimated: gameobject 9900730 is absent from every archive, so the crate borrows the Excavation Supply Crate
-- display and lock; no Defias Bandit or Defias Rogue Wizard is recorded at the Bastion, so their spawns fill
-- free ground in the camp; the Suspicious Guard uses the Defias faction because his orders drop on death.
-- Reward items missing from this world are skipped by the quest loader.

-- Quest items
REPLACE INTO `item_template` (`entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`, `Quality`, `Flags`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`, `maxcount`, `stackable`, `spellcooldown_1`, `spellcategorycooldown_1`, `spellcooldown_2`, `spellcategorycooldown_2`, `spellcooldown_3`, `spellcategorycooldown_3`, `spellcooldown_4`, `spellcategorycooldown_4`, `spellcooldown_5`, `spellcategorycooldown_5`, `bonding`, `startquest`, `Material`, `RequiredDisenchantSkill`, `VerifiedBuild`) VALUES
(5055564, 12, 0, -1, 'Stolen Goods', 10090, 1, 2048, -1, -1, 0, 0, 0, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4, 0, -1, -1, 0),
(5055565, 12, 0, -1, 'Tattered Orders', 5061, 1, 2048, -1, -1, 1, 8, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4, 100074, -1, -1, 0);

-- Defias Disruption, Supply Run and A Betrayal Within
REPLACE INTO `quest_template` (`ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestSortID`, `RewardXPDifficulty`, `RewardMoney`, `RewardMoneyDifficulty`, `StartItem`, `RewardItem1`, `RewardAmount1`, `RewardItem2`, `RewardAmount2`, `RewardChoiceItemID1`, `RewardChoiceItemQuantity1`, `RewardChoiceItemID2`, `RewardChoiceItemQuantity2`, `RewardChoiceItemID3`, `RewardChoiceItemQuantity3`, `LogTitle`, `LogDescription`, `QuestDescription`, `QuestCompletionLog`, `RequiredNpcOrGo1`, `RequiredNpcOrGo2`, `RequiredNpcOrGoCount1`, `RequiredNpcOrGoCount2`, `RequiredItemId1`, `RequiredItemCount1`, `VerifiedBuild`) VALUES
(100071, 2, 8, 4, 12, 5, 130, 135, 0, 1397885, 1, 500813, 1, 0, 0, 0, 0, 0, 0, 'Defias Disruption', 'Thin the ranks of the Defias at the Bandit Bastion, east of Goldshire.', 'Oh, um... are you one of those brave types? The ones who deal with danger and monsters and... all that?  It’s just that things have not been the same here lately. The taproom’s quieter, the roads feel emptier, and even the usual loud sorts have stopped passing through. I keep trying to tell myself it is just a slow season, but... I do not think that''s it.  I heard a rumor, just a rumor, that some of those Defias bandits have built up a camp somewhere east of here. People are saying they''ve been stopping travelers, stealing supplies, or maybe worse. I do not really know for sure, but... maybe if someone went out there and gave them a reason to think twice, things might calm down a bit?  That might bring folks back.', 'Return to Melika Isenstrider at Goldshire in Elwynn Forest.', 116, 474, 6, 4, 0, 0, 0),
(100073, 2, 8, 5, 12, 5, 0, 90, 0, 1397885, 1, 0, 0, 500814, 1, 500815, 1, 500816, 1, 'Supply Run', 'Recover 4 Stolen Supply Crates from the Bandit Bastion, east of Goldshire.', 'Listen, I have a bit of a follow up for you, if you''re still feeling up to the task.  There’s talk that the Defias have gathered in a proper hideout east of here. It’s a large encampment, far more organized than the usual rabble.  Some local merchants were hit in a raid not long ago. Crates of supplies were taken. Useful things like tools, cloth, even some rare spirits. The sort of goods folks rely on around here.  It’s not without risk, but if someone were to head in and recover what was lost, I know a few people who would be grateful. I would see to it that you''re rewarded for your effort.', 'Return to Remy "Two Times" at Goldshire in Elwynn Forest.', 0, 0, 0, 0, 5055564, 4, 0),
(100074, 2, 8, 5, 12, 5, 130, 135, 5055565, 1397884, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'A Betrayal Within', 'Return to Marshal Dughan.', 'You''ve found what looks like a genuine Alliance document, carried by a man dressed as a soldier stationed deep in a Defias outpost. The contents point to collusion between Alliance forces and local bandits.  This should not be happening. Someone loyal needs to see this.  Deliver the document to an appropriate Alliance official. With any luck, they will know what to do with it.', 'Return to Marshal Dughan at Goldshire in Elwynn Forest.', 0, 0, 0, 0, 0, 0, 0);
DELETE FROM `creature_queststarter` WHERE `quest` IN (100071, 100073);
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES
(6778, 100071),
(241, 100073);
DELETE FROM `creature_questender` WHERE `quest` IN (100071, 100073, 100074);
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(6778, 100071),
(241, 100073),
(240, 100074);
DELETE FROM `quest_template_addon` WHERE `ID` IN (100071, 100073);
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(100071, 47),
(100073, 47);

-- Suspicious Guard, carrying the Tattered Orders
REPLACE INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(991516, 0, 0, 0, 0, 0, 'Suspicious Guard', NULL, NULL, 0, 8, 8, 0, 17, 0, 1, 1.14286, 1, 1, 18, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 7, 0, 991516, 0, 0, 0, 0, 0, 0, '', 0, 1, 1.44, 1, 1, 1, 0, 0, 1, 0, 0, '', 0);
DELETE FROM `creature_template_model` WHERE `CreatureID` = 991516;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES (991516, 0, 1984, 1, 1, 0);
DELETE FROM `creature_loot_template` WHERE `Entry` = 991516;
INSERT INTO `creature_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(991516, 5055565, 0, 100, 0, 1, 0, 1, 1, 'Suspicious Guard - Tattered Orders');

-- Stolen Supply Crates
REPLACE INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `size`, `Data0`, `Data1`, `Data3`, `Data8`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(9900730, 3, 31, 'Stolen Supply Crate', '', '', 1, 43, 9900730, 1, 100073, '', '', 0);
DELETE FROM `gameobject_loot_template` WHERE `Entry` = 9900730;
INSERT INTO `gameobject_loot_template` (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(9900730, 5055564, 0, 100, 1, 1, 0, 1, 1, 'Stolen Supply Crate - Stolen Goods');

-- The Suspicious Guard, Defias Bandits and Defias Rogue Wizards at the Bandit Bastion
DELETE FROM `creature` WHERE `guid` BETWEEN 9001029 AND 9001041;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`) VALUES
(9001029, 991516, 0, 0, 0, 1, 1, 0, -9791.926, -481.764, 29.191, 1.65585, 120, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001030, 116, 0, 0, 0, 1, 1, 1, -9794.6, -450.4, 29.598, 0, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001031, 474, 0, 0, 0, 1, 1, 1, -9786.6, -450.4, 30.337, 3.14159, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001032, 116, 0, 0, 0, 1, 1, 1, -9802.6, -452.4, 29.467, 0.24498, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001033, 474, 0, 0, 0, 1, 1, 1, -9800.6, -444.4, 29.862, 5.49779, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001034, 116, 0, 0, 0, 1, 1, 1, -9796.6, -458.4, 29.495, 1.32582, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001035, 116, 0, 0, 0, 1, 1, 1, -9782.6, -442.4, 30.522, 3.7296, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001036, 474, 0, 0, 0, 1, 1, 1, -9778.6, -450.4, 31.214, 3.14159, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001037, 116, 0, 0, 0, 1, 1, 1, -9794.6, -466.4, 29.086, 1.5708, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001038, 474, 0, 0, 0, 1, 1, 1, -9794.6, -432.4, 29.819, 4.71239, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001039, 116, 0, 0, 0, 1, 1, 1, -9782.6, -434.4, 31.169, 4.06889, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001040, 116, 0, 0, 0, 1, 1, 1, -9812.6, -438.4, 29.758, 5.69518, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content'),
(9001041, 474, 0, 0, 0, 1, 1, 1, -9786.6, -470.4, 29.225, 1.9513, 180, 3, 0, 0, 0, 1, 0, 0, 0, '', 0, 0, 'Goldshire WB1 content');

-- Stolen Supply Crates in the camp
DELETE FROM `gameobject` WHERE `guid` BETWEEN 9001060 AND 9001068;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`, `Comment`) VALUES
(9001060, 9900730, 0, 0, 0, 1, 1, -9804.89, -434.225, 29.773, 0, 0, 0, 0, 1, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001061, 9900730, 0, 0, 0, 1, 1, -9790.074, -439.43, 29.697, 2.39996, 0, 0, 0.9320318, 0.3623764, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001062, 9900730, 0, 0, 0, 1, 1, -9822.716, -438.736, 29.85, 4.79992, 0, 0, 0.6754927, -0.7373667, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001063, 9900730, 0, 0, 0, 1, 1, -9815.076, -446.717, 29.613, 0.91669, 0, 0, 0.4424666, 0.896785, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001064, 9900730, 0, 0, 0, 1, 1, -9784.749, -461.985, 30.815, 3.31665, 0, 0, 0.9961716, -0.0874193, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001065, 9900730, 0, 0, 0, 1, 1, -9803.038, -469.966, 28.886, 5.71661, 0, 0, 0.2795115, -0.9601423, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001066, 9900730, 0, 0, 0, 1, 1, -9811.14, -456.433, 29.119, 1.83339, 0, 0, 0.7935949, 0.6084466, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001067, 9900730, 0, 0, 0, 1, 1, -9769.239, -443.594, 31.83, 4.23335, 0, 0, 0.8546716, -0.519169, 60, 100, 1, '', 0, 'Goldshire WB1 content'),
(9001068, 9900730, 0, 0, 0, 1, 1, -9757.664, -442.206, 32.799, 0.35012, 0, 0, 0.1741692, 0.9847157, 60, 100, 1, '', 0, 'Goldshire WB1 content');
