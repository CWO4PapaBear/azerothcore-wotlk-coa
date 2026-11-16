DELETE FROM `spell_proc` WHERE `SpellId` IN (84510, 965503, 965504, 965505, 81119, 81122, 81479, 965315, 84748, 983908);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(84510, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 1, 0, 33, 4000, 0),
(965503, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 3, 0, 33, 6000, 0),
(965504, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 33, 12000, 0),
(965505, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 33, 0, 0),
(81119, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(81122, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(81479, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(965315, 0, 0, 0, 0, 0, 327952, 1, 2, 0, 2, 5, 0, 100, 0, 0),
(84748, 0, 0, 0, 0, 0, 87312, 7, 1, 0, 0, 3, 0, 100, 0, 1),
(983908, 0, 0, 0, 0, 0, 87312, 7, 1, 0, 0, 3, 0, 100, 0, 2);
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_legendary_batch', 'spell_area52_slinger_consumption', 'spell_area52_bone_chill_vulnerability');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(84510, 'spell_area52_legendary_batch'),
(965503, 'spell_area52_legendary_batch'),
(965504, 'spell_area52_legendary_batch'),
(965505, 'spell_area52_legendary_batch'),
(81119, 'spell_area52_legendary_batch'),
(81122, 'spell_area52_legendary_batch'),
(81479, 'spell_area52_legendary_batch'),
(965315, 'spell_area52_legendary_batch'),
(84748, 'spell_area52_slinger_consumption'),
(983908, 'spell_area52_slinger_consumption'),
(965706, 'spell_area52_bone_chill_vulnerability');

DELETE FROM `spell_proc` WHERE `SpellId` = 99634;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(99634, 0, 0, 0, 0, 0, 20, 1, 2, 0, 0, 6, 0, 100, 0, 0);
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_corrupted_bear', 'spell_area52_corrupted_bear_form');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(99634, 'spell_area52_corrupted_bear'),
(5487, 'spell_area52_corrupted_bear_form'),
(9634, 'spell_area52_corrupted_bear_form');
DELETE FROM `spell_bonus_data` WHERE `entry` IN (84511, 965509, 965510, 965316);
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(84511, 0.25, 0, 0.18, 0, 'Area52 Unbounded Spell Slinger'),
(965509, 0.7, 0, 0.5, 0, 'Area52 Spell Slinger'),
(965510, 0.3656, 0, 0.2376, 0, 'Area52 Frost Lich root tooltip contract'),
(965316, 0, 0.015, 0, 0.01035, 'Area52 Wild Felfire per tick');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_felfire_equipment', 'spell_area52_pure_chaos_bolt');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(965315, 'spell_area52_felfire_equipment'),
(944811, 'spell_area52_pure_chaos_bolt'),
(944812, 'spell_area52_pure_chaos_bolt'),
(944813, 'spell_area52_pure_chaos_bolt'),
(944814, 'spell_area52_pure_chaos_bolt');
DELETE FROM `spell_ranks` WHERE `spell_id` IN (944811, 944812, 944813, 944814);
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(944811, 944811, 1),
(944811, 944812, 2),
(944811, 944813, 3),
(944811, 944814, 4);

DELETE FROM `spell_proc` WHERE `SpellId` = 954046;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(954046, 24, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 2, 0, 100, 0, 0);
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_overloaded_mind', 'spell_area52_froststorm_echo');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(954046, 'spell_area52_overloaded_mind'),
(954048, 'spell_area52_froststorm_echo');
DELETE FROM `spell_bonus_data` WHERE `entry` IN (954048, 954049);
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(954048, 0.0667, 0, 0.0434, 0, 'Area52 Overloaded Froststorm'),
(954049, 0.11, 0, 0.71, 0, 'Area52 Astral Tempest');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_felfire_emberstorm';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(411119, 'spell_area52_felfire_emberstorm');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_corrupted_bear_instant';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(17941, 'spell_area52_corrupted_bear_instant'),
(34936, 'spell_area52_corrupted_bear_instant');
