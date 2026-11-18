DELETE FROM `spell_script_names` WHERE `ScriptName` IN
('spell_area52_embracing_dance', 'aura_area52_embracing_shadows', 'spell_area52_embracing_extension');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(51713, 'spell_area52_embracing_dance'),
(414100, 'aura_area52_embracing_shadows'),
(414101, 'spell_area52_embracing_extension');
DELETE FROM `spell_proc` WHERE `SpellId` = 414100;
INSERT INTO `spell_proc` (`SpellId`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `Chance`)
VALUES (414100, 1048576, 1, 2, 100);
