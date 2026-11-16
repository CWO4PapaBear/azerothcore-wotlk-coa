DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('aura_area52_eldritch_knight',
'spell_area52_eldritch_missing_mana', 'spell_area52_eldritch_condemnation');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(81116, 'aura_area52_eldritch_knight'),
(983708, 'spell_area52_eldritch_missing_mana'),
(983712, 'spell_area52_eldritch_missing_mana'),
(983703, 'spell_area52_eldritch_condemnation'),
(983704, 'spell_area52_eldritch_condemnation'),
(983705, 'spell_area52_eldritch_condemnation'),
(983706, 'spell_area52_eldritch_condemnation'),
(983707, 'spell_area52_eldritch_condemnation');

DELETE FROM `spell_proc` WHERE `SpellId` = 81116;
INSERT INTO `spell_proc` (`SpellId`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`,
`DisableEffectsMask`, `Chance`) VALUES
(81116, 20, 1, 2, 3, 4, 100);
