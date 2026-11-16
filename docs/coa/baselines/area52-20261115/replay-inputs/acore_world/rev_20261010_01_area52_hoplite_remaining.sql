DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_hunger_for_blood', 'aura_area52_wind_slam', 'spell_area52_wind_slam_damage', 'aura_area52_draconic_sinister');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(51662, 'spell_area52_hunger_for_blood'),
(979187, 'aura_area52_wind_slam'),
(979188, 'aura_area52_wind_slam'),
(979189, 'spell_area52_wind_slam_damage'),
(782246, 'aura_area52_draconic_sinister');
DELETE FROM `spell_proc` WHERE `SpellId` IN (979187, 979188, 782246);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(979187, 0, 11, 8388608, 0, 0, 16, 1, 2, 0, 2, 2, 0, 100, 0, 0),
(979188, 0, 0, 0, 0, 0, 69904, 1, 2, 0, 2, 0, 0, 100, 0, 0),
(782246, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0);
DELETE FROM `spell_group` WHERE `id` = 1059 AND `spell_id` = 979189;
INSERT INTO `spell_group` (`id`, `spell_id`) VALUES
(1059, 979189);
