DELETE FROM `spell_proc` WHERE `SpellId` = 81501;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(81501, 8, 7, 96, 0, 0, 16384, 2, 2, 3, 0, 0, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 81501 AND `ScriptName` = 'spell_area52_evergreen';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(81501, 'spell_area52_evergreen');

DELETE FROM `spell_script_names` WHERE (`spell_id` = 81249 AND `ScriptName` = 'spell_area52_call_fungarians') OR (`spell_id` = 81413 AND `ScriptName` = 'aura_area52_cloud_of_rot');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(81249, 'spell_area52_call_fungarians'),
(81413, 'aura_area52_cloud_of_rot');

DELETE FROM `spell_bonus_data` WHERE `entry` = 81413;
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(81413, 0, 0, 0, 0, 'Area 52 - Cloud of Rot uses summoner stats in native aura calculation');

UPDATE `creature_template` SET `ScriptName` = 'npc_ascension_summoned_effect' WHERE `entry` = 43286;

DELETE FROM `spell_proc` WHERE `SpellId` = 415012;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(415012, 8, 0, 0, 0, 0, 279552, 2, 2, 3, 2, 2, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE (`spell_id` = 415012 AND `ScriptName` = 'aura_area52_everliving') OR (`spell_id` = 415013 AND `ScriptName` = 'aura_area52_everliving_stacks') OR (`spell_id` = 18562 AND `ScriptName` = 'spell_area52_everliving_swiftmend');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(415012, 'aura_area52_everliving'),
(415013, 'aura_area52_everliving_stacks'),
(18562, 'spell_area52_everliving_swiftmend');
