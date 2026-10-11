DELETE FROM `spell_ranks` WHERE `first_spell_id` IN (20243, 53385) OR `spell_id` IN (20243, 302035, 302036, 302037, 302038, 30016, 30022, 47497, 47498, 53385, 54762, 54763, 54764, 54765, 54766, 54767);
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(20243, 20243, 1),
(20243, 302035, 2),
(20243, 302036, 3),
(20243, 302037, 4),
(20243, 302038, 5),
(20243, 30016, 6),
(20243, 30022, 7),
(20243, 47497, 8),
(20243, 47498, 9),
(53385, 53385, 1),
(53385, 54762, 2),
(53385, 54763, 3),
(53385, 54764, 4),
(53385, 54765, 5),
(53385, 54766, 6),
(53385, 54767, 7);

DELETE FROM `spell_proc` WHERE `SpellId` = 965862;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(965862, 0, 0, 0, 0, 0, 56, 1, 2, 34, 0, 6, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('aura_area52_blademaster', 'spell_area52_blademaster_empower', 'aura_area52_tactical_mastery', 'aura_area52_tactical_stagger', 'aura_area52_tactical_debt');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(965862, 'aura_area52_blademaster'),
(-20243, 'spell_area52_blademaster_empower'),
(14251, 'spell_area52_blademaster_empower'),
(-53385, 'spell_area52_blademaster_empower'),
(12295, 'aura_area52_tactical_mastery'),
(12676, 'aura_area52_tactical_mastery'),
(12677, 'aura_area52_tactical_mastery'),
(20231, 'aura_area52_tactical_stagger'),
(20232, 'aura_area52_tactical_debt');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (53385, -53385, 54762, 54763, 54764, 54765, 54766, 54767) AND `ScriptName` = 'spell_pal_divine_storm';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-53385, 'spell_pal_divine_storm');
