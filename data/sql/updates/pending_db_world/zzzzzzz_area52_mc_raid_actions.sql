START TRANSACTION;
INSERT INTO `creature_template`
(`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`,
`subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`,
`speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`,
`BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`,
`dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`,
`VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`,
`ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`,
`flags_extra`, `ScriptName`)
VALUES
(92030, 0, 0, 0, 92030, 92030, 'Sacrificial Chains', NULL, NULL, 0, 63, 63, 0, 54, 0, 1, 1.14286, 1, 1, 20, 1, 0, 1,
2000, 2000, 1, 1, 1, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
'npc_sacrificial_chains_coa');
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`)
VALUES
(92030, 0, 20367, 1, 1);
INSERT INTO `coa_boss_flex` (`entry`, `hp_d0`, `hp_d1`, `hp_d2`, `hp_d3`, `comment`) SELECT
92030, 8625, 11501, 19553, 32147, 'Sacrificial Chains: Normal = Heroic x 0.750 (set), Heroic/Mythic/Ascended measured (mc-dataset.json kill health / player count)' WHERE NOT EXISTS (SELECT 1 FROM coa_boss_flex WHERE entry=92030 AND hp_d0=8625 AND hp_d1=11501 AND hp_d2=19553 AND hp_d3=32147);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(11673, 0, 3, 0, 0, 0, 100, 0, 8000, 8000, 15000, 15000, 0, 0, 11, 2105025, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ancient Core Hound - In Combat - Cast \'Melt Armor\' (Phase 1) (No Repeat) (All Difficulties)'),
(11673, 0, 4, 0, 0, 0, 100, 24, 8000, 8000, 40000, 40000, 0, 0, 11, 2105309, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ancient Core Hound - In Combat - Cast \'Panic\' Fear (Phase 1) (No Repeat) (Mythic+Ascended Only)');
UPDATE gameobject_template SET ScriptName='go_ragnaros_portal_coa',
    `type` = 10,
    `displayId` = 7161,
    `name` = 'Portal to Ragnaros\' Lair',
    `size` = 1,
    `Data0` = 0, `Data1` = 0, `Data2` = 0, `Data3` = 0, `Data4` = 0, `Data5` = 0, `Data6` = 0,
    `Data7` = 0, `Data8` = 0, `Data9` = 0, `Data10` = 0, `Data11` = 0, `Data12` = 0, `Data13` = 0,
    `Data14` = 0, `Data15` = 0, `Data16` = 0, `Data17` = 0, `Data18` = 0, `Data19` = 0, `Data20` = 0
WHERE entry=181623 AND type=5 AND displayId=6450 AND ScriptName='';

UPDATE smart_scripts SET event_flags=0 WHERE entryorguid IN (11658,11659,11661,11666,11667,11668) AND source_type=0 AND event_flags=2 AND action_type=11 AND action_param1 IN (18944,18945,20276,19129,19730,20277,15502,19635,19636,19630,19631,19393,19392);
UPDATE creature_template SET ScriptName='boss_garr_coa' WHERE entry IN (12057,112057,212057,312057) AND ScriptName='coa_boss_ai';
INSERT INTO spelldifficulty_dbc (ID,DifficultySpellID_1,DifficultySpellID_2,DifficultySpellID_3,DifficultySpellID_4) VALUES (72450,2105509,2105510,2105511,2105512);
INSERT INTO spell_script_names (spell_id,ScriptName) VALUES (2108607,'spell_ragnaros_magma_blast_coa_dummy');
COMMIT;
