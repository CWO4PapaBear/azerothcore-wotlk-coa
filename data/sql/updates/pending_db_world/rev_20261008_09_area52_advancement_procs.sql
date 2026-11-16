DELETE FROM `spell_proc` WHERE `SpellId` IN (12753,12308,12810,12811,14142,14769,816968,31682,31683,31684,33225);
INSERT INTO `spell_proc` (`SpellId`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`ProcFlags`,`SpellTypeMask`,`SpellPhaseMask`,`HitMask`,`AttributesMask`,`DisableEffectsMask`,`ProcsPerMinute`,`Chance`,`Cooldown`,`Charges`) VALUES
(12753,0,0,0,0,0,40,0,2,16,0,0,0,0,0,0),
(12308,0,0,0,0,0,69648,1,2,0,0,0,0,0,0,0),
(12810,0,0,0,0,0,69648,1,2,0,0,0,0,0,0,0),
(12811,0,0,0,0,0,69648,1,2,0,0,0,0,0,0,0),
(14142,0,0,0,0,0,262144,1,2,0,0,0,0,100,0,0),
(14769,0,0,0,0,0,17408,4,2,0,0,0,0,100,0,0),
(816968,0,0,0,0,0,16,1,2,0,0,0,0,100,0,0),
(31682,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(31683,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(31684,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(33225,0,0,0,0,0,262144,1,2,0,0,0,0,30,0,0);
DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_advancement_proc' AND `spell_id` IN (12753,12308,12810,12811,14142,14769,816968,31682,31683,31684,33225);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(12753,'spell_area52_advancement_proc'),
(12308,'spell_area52_advancement_proc'),
(12810,'spell_area52_advancement_proc'),
(12811,'spell_area52_advancement_proc'),
(14142,'spell_area52_advancement_proc'),
(14769,'spell_area52_advancement_proc'),
(816968,'spell_area52_advancement_proc'),
(31682,'spell_area52_advancement_proc'),
(31683,'spell_area52_advancement_proc'),
(31684,'spell_area52_advancement_proc'),
(33225,'spell_area52_advancement_proc');
