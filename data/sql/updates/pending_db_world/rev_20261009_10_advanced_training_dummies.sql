INSERT INTO `creature_template` (`entry`, `name`, `minlevel`, `maxlevel`, `exp`, `faction`, `rank`, `unit_class`, `type`, `HealthModifier`, `ArmorModifier`, `DamageModifier`, `RegenHealth`, `ScriptName`) VALUES
(666925, 'Dynamic Execute Training Dummy', 1, 1, 0, 7, 1, 1, 10, 1000, 1, 1, 0, 'npc_advanced_training_dummy'),
(666935, 'Dynamic Healing Training Dummy', 1, 1, 0, 35, 1, 1, 7, 1000, 1, 1, 0, 'npc_advanced_training_dummy'),
(967171, 'Configurable Training Dummy', 1, 1, 0, 7, 1, 1, 10, 1000, 1, 1, 0, 'npc_advanced_training_dummy'),
(967182, 'Dynamic Tank Training Dummy', 1, 1, 0, 7, 1, 1, 10, 1000, 1, 1, 0, 'npc_advanced_training_dummy'),
(967254, 'Dynamic Training Dummy', 1, 1, 0, 7, 1, 1, 10, 1000, 1, 1, 0, 'npc_advanced_training_dummy') ON DUPLICATE KEY UPDATE `ScriptName` = VALUES(`ScriptName`);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (666925, 666935, 967171, 967182, 967254);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(666925, 0, 28047, 1, 1),
(666935, 0, 1555, 1, 1),
(967171, 0, 28047, 1, 1),
(967182, 0, 28047, 1, 1),
(967254, 0, 28047, 1, 1);
