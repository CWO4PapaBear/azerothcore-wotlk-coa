UPDATE `spell_proc` SET `ProcFlags` = 272
WHERE `SpellId` = -34500 AND `ProcFlags` = 0 AND `SpellFamilyName` = 0
  AND `SpellTypeMask` = 1 AND `SpellPhaseMask` = 2 AND `HitMask` = 2;

DELETE FROM `spell_script_names`
WHERE `spell_id` = -34500 AND `ScriptName` = 'spell_hun_area52_expose_weakness';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-34500, 'spell_hun_area52_expose_weakness');
