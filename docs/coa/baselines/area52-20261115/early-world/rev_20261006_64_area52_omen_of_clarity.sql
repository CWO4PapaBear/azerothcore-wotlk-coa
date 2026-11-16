DELETE FROM `spell_proc` WHERE `SpellId` IN (-16864, 16864, 816860, 816861);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(16864, 0, 0, 0, 0, 0, 349524, 3, 2, 0, 0, 6, 0, 2, 2000, 0),
(816860, 0, 0, 0, 0, 0, 349524, 3, 2, 0, 0, 6, 0, 4, 2000, 0),
(816861, 0, 0, 0, 0, 0, 349524, 3, 2, 0, 0, 6, 0, 6, 2000, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (16864, -16864, 816860, 816861) AND `ScriptName` = 'spell_dru_omen_of_clarity';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(16864, 'spell_dru_omen_of_clarity'),
(816860, 'spell_dru_omen_of_clarity'),
(816861, 'spell_dru_omen_of_clarity');
