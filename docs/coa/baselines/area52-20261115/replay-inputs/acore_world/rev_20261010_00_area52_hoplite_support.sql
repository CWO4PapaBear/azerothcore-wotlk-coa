DELETE FROM `spell_script_names` WHERE `spell_id` IN (-12317, 12317, 13045, 13046, 13047, 13048) AND `ScriptName` = 'spell_gen_proc_from_direct_damage';
DELETE FROM `spell_script_names` WHERE `spell_id` IN (-51698, 51698, 51700, 51701) AND `ScriptName` = 'spell_rog_honor_among_thieves';
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_hoplite_support_proc';
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_sweeping_swings';
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_shattering_throw';
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_sweeping_swings_damage';
DELETE FROM `spell_script_names` WHERE `spell_id` = 64380 AND `ScriptName` = 'spell_warr_shattering_throw';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(12317, 'spell_area52_hoplite_support_proc'),
(13045, 'spell_area52_hoplite_support_proc'),
(13046, 'spell_area52_hoplite_support_proc'),
(13047, 'spell_area52_hoplite_support_proc'),
(13048, 'spell_area52_hoplite_support_proc'),
(51701, 'spell_area52_hoplite_support_proc'),
(84445, 'spell_area52_hoplite_support_proc'),
(812297, 'spell_area52_hoplite_support_proc'),
(912326, 'spell_area52_hoplite_support_proc'),
(912326, 'aura_area52_sweeping_swings'),
(64380, 'spell_area52_shattering_throw'),
(12328, 'aura_area52_sweeping_swings'),
(997604, 'spell_area52_sweeping_swings_damage');
DELETE FROM `spell_proc` WHERE `SpellId` IN (-12317, 12317, 13045, 13046, 13047, 13048, -51698, 51698, 51700, 51701, -84445, 84445);
DELETE FROM `spell_proc` WHERE `SpellId` IN (-812297, 812297);
DELETE FROM `spell_proc` WHERE `SpellId` IN (-912326, 912326);
DELETE FROM `spell_proc` WHERE `SpellId` IN (-12834, 12834, 12849, 12867);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(12317, 0, 0, 0, 0, 0, 17408, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(13045, 0, 0, 0, 0, 0, 17408, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(13046, 0, 0, 0, 0, 0, 17408, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(13047, 0, 0, 0, 0, 0, 17408, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(13048, 0, 0, 0, 0, 0, 157352, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(51701, 0, 0, 0, 0, 0, 69904, 1, 2, 0, 0, 6, 0, 100, 3000, 0),
(84445, 0, 0, 0, 0, 0, 69648, 7, 2, 0, 0, 0, 0, 100, 20000, 0),
(812297, 0, 0, 0, 0, 0, 69648, 7, 2, 0, 0, 3, 0, 100, 0, 0),
(912326, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 5, 0, 100, 1000, 0),
(12834, 0, 0, 0, 0, 0, 20, 1, 2, 2, 0, 0, 0, 100, 0, 0),
(12849, 0, 0, 0, 0, 0, 20, 1, 2, 2, 0, 0, 0, 100, 0, 0),
(12867, 0, 0, 0, 0, 0, 20, 1, 2, 2, 0, 0, 0, 100, 0, 0);
