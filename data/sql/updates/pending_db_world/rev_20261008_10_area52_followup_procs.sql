DELETE FROM `spell_proc` WHERE `SpellId` IN (16288,63506,17958,34493,12318,12857,16219,863372);
INSERT INTO `spell_proc` (`SpellId`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`ProcFlags`,`SpellTypeMask`,`SpellPhaseMask`,`HitMask`,`AttributesMask`,`DisableEffectsMask`,`ProcsPerMinute`,`Chance`,`Cooldown`,`Charges`) VALUES
(16288,0,0,0,0,0,17408,0,1,0,0,0,0,100,20000,0),
(63506,0,0,0,0,0,17408,0,1,0,0,0,0,100,0,0),
(17958,0,0,0,0,0,69632,0,1,0,0,4,0,100,0,0),
(34493,0,0,0,0,0,262144,1,2,0,0,0,0,100,1000,0),
(12318,0,0,0,0,0,17408,0,1,0,0,0,0,100,0,0),
(12857,0,0,0,0,0,17408,0,1,0,0,0,0,100,0,0),
(16219,0,0,0,0,0,17408,2,2,2,0,0,0,100,0,0),
(863372,0,0,0,0,0,65536,1,2,0,0,0,0,100,0,0);
DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_followup_proc' AND `spell_id` IN (16288,63506,17958,34493,12318,12857,16219,863372);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(16288,'spell_area52_followup_proc'),
(63506,'spell_area52_followup_proc'),
(17958,'spell_area52_followup_proc'),
(34493,'spell_area52_followup_proc'),
(12318,'spell_area52_followup_proc'),
(12857,'spell_area52_followup_proc'),
(16219,'spell_area52_followup_proc'),
(863372,'spell_area52_followup_proc');
