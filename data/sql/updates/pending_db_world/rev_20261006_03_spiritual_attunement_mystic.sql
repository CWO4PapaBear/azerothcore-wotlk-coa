DELETE FROM `spell_script_names` WHERE `spell_id` = 965377 AND `ScriptName` = 'spell_pal_spiritual_attunement';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (965377, 'spell_pal_spiritual_attunement');

DELETE FROM `spell_proc` WHERE `SpellId` = 965377;
INSERT INTO `spell_proc` (`SpellId`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `Chance`)
VALUES (965377, 559104, 2, 2, 3, 100);
