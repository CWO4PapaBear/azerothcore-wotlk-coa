DELETE FROM `spell_proc` WHERE `SpellId` = 81258;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(81258, 0, 4, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 4000, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_battlemage', 'spell_area52_battlemage_damage');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
SELECT 81258, 'spell_area52_battlemage'
UNION SELECT `spell_id`, 'spell_area52_battlemage_damage' FROM `spell_ranks` WHERE `first_spell_id` IN (44425, 2136, 30455)
UNION SELECT 44425, 'spell_area52_battlemage_damage'
UNION SELECT 2136, 'spell_area52_battlemage_damage'
UNION SELECT 30455, 'spell_area52_battlemage_damage';
