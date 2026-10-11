DELETE FROM `spell_script_names` WHERE `spell_id` = 84445 AND `ScriptName` IN ('spell_area52_hoplite_support_proc', 'aura_area52_spartan_kick');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (84445, 'aura_area52_spartan_kick');

DELETE FROM `spell_proc` WHERE `SpellId` IN (-84445, 84445);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(84445, 0, 0, 0, 0, 0, 69648, 7, 2, 0, 0, 0, 0, 100, 20000, 0);
