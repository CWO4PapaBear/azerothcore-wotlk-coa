INSERT IGNORE INTO `creature_template`
(`entry`, `name`, `minlevel`, `maxlevel`, `faction`, `unit_class`, `type`, `BaseAttackTime`, `RangeAttackTime`, `HealthModifier`, `ManaModifier`, `unit_flags`, `flags_extra`, `ScriptName`)
SELECT 300055, 'Consecrated Ground', 1, 1, 35, 1, 10, 2000, 2000, 1, 1, 33554438, 128, 'npc_area52_consecrated_ground'
WHERE NOT EXISTS (SELECT 1 FROM `creature_template` WHERE `entry` = 300055);

INSERT IGNORE INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`)
SELECT 300055, 0, 200003, 1, 1
WHERE EXISTS (SELECT 1 FROM `creature_template` WHERE `entry` = 300055 AND `ScriptName` = 'npc_area52_consecrated_ground')
AND NOT EXISTS (SELECT 1 FROM `creature_template_model` WHERE `CreatureID` = 300055);
