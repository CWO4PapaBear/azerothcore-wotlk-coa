DELETE FROM `spell_script_names` WHERE `spell_id` IN (-59088, 59088, 59089, 859089) AND `ScriptName` = 'spell_warr_improved_spell_reflection';
DELETE FROM `spell_proc` WHERE `SpellId` IN (-59088, 59088, 59089, 859089, 977828, 978724, 922565, 86378);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(977828, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(978724, 0, 0, 0, 0, 0, 680, 1, 0, 8256, 0, 0, 0, 100, 1000, 0),
(922565, 0, 0, 0, 0, 0, 139944, 1, 0, 9283, 0, 6, 0, 100, 0, 0),
(86378, 0, 0, 0, 0, 0, 139944, 1, 0, 9283, 0, 0, 0, 100, 0, 0);

DELETE FROM `spell_proc` WHERE `SpellId` IN (-20127, 20127, 20130, 20135, 20100);
INSERT INTO `spell_proc` (`SpellId`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `DisableEffectsMask`, `Chance`, `Cooldown`) VALUES
(20127, 680, 1, 0, 8256, 2, 10, 0),
(20130, 680, 1, 0, 8256, 2, 10, 0),
(20135, 680, 1, 0, 8256, 2, 10, 0),
(20100, 680, 1, 0, 8256, 5, 100, 1000);

UPDATE `spell_bonus_data` SET `ap_bonus` = 0.69 WHERE `entry` = 1495;

DELETE FROM `spell_proc` WHERE `SpellId` = 20095;
INSERT INTO `spell_proc` (`SpellId`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `DisableEffectsMask`, `Chance`, `Charges`) VALUES
(20095, 680, 1, 0, 8256, 0, 100, 2);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_shield_tools', 'aura_area52_bulwark_cleanup', 'aura_area52_wild_aggression', 'aura_area52_bulwark_energy', 'spell_area52_bulwark_channel', 'aura_area52_bulwark_drain', 'spell_area52_shockwave', 'spell_area52_shockwave_damage', 'spell_area52_bulwark_damage', 'aura_area52_combat_readiness');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(871, 'spell_area52_shield_tools'),
(2565, 'spell_area52_shield_tools'),
(23920, 'spell_area52_shield_tools'),
(-23922, 'spell_area52_shield_tools'),
(859089, 'aura_area52_bulwark_cleanup'),
(986125, 'aura_area52_bulwark_cleanup'),
(977828, 'aura_area52_bulwark_cleanup'),
(901144, 'aura_area52_bulwark_cleanup'),
(978724, 'aura_area52_bulwark_cleanup'),
(977828, 'aura_area52_wild_aggression'),
(978724, 'aura_area52_bulwark_energy'),
(922565, 'spell_area52_bulwark_channel'),
(923923, 'aura_area52_bulwark_drain'),
(46968, 'spell_area52_shockwave'),
(846968, 'spell_area52_shockwave_damage'),
(846969, 'spell_area52_shockwave_damage'),
(923921, 'spell_area52_bulwark_damage'),
(923922, 'spell_area52_bulwark_damage'),
(86378, 'aura_area52_combat_readiness'),
(886378, 'aura_area52_combat_readiness');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_intervene_taunt', 'spell_area52_primal_precision');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(3411, 'spell_area52_intervene_taunt'),
(-1079, 'spell_area52_primal_precision'),
(-22568, 'spell_area52_primal_precision'),
(-22570, 'spell_area52_primal_precision'),
(52610, 'spell_area52_primal_precision');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_tank_stance';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(71, 'aura_area52_tank_stance'),
(48263, 'aura_area52_tank_stance'),
(25780, 'aura_area52_tank_stance'),
(701463, 'aura_area52_tank_stance'),
(5487, 'aura_area52_tank_stance'),
(9634, 'aura_area52_tank_stance');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_avatar_movement';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (86380, 'aura_area52_avatar_movement');
