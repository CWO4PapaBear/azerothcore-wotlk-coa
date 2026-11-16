DELETE FROM `spell_proc` WHERE `SpellId` IN (20361,20362,811100,20042,20045,851561,44400,44402,44403,760191,19298);
INSERT INTO `spell_proc` (`SpellId`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`ProcFlags`,`SpellTypeMask`,`SpellPhaseMask`,`HitMask`,`AttributesMask`,`DisableEffectsMask`,`ProcsPerMinute`,`Chance`,`Cooldown`,`Charges`) VALUES
(20361,0,0,0,0,0,69632,0,1,0,0,0,0,100,0,0),
(20362,0,0,0,0,0,87040,0,4,0,8,0,0,100,0,1),
(811100,0,0,0,0,0,65536,0,4,0,8,0,0,100,0,1),
(20042,0,0,0,0,0,17408,0,2,0,0,2,0,100,0,0),
(20045,0,0,0,0,0,17408,0,2,0,0,2,0,100,0,0),
(851561,0,0,0,0,0,17408,2,2,0,2,0,0,100,5000,0),
(44400,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(44402,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(44403,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(760191,0,0,0,0,0,17408,2,2,0,0,0,0,100,0,0),
(19298,0,0,0,0,0,16,1,2,2,0,0,0,100,4000,0);
DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_followup_proc' AND `spell_id` IN (20361,811100,20042,20045,851561,44400,44402,44403,760191,19298);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(20361,'spell_area52_followup_proc'),
(811100,'spell_area52_followup_proc'),
(20042,'spell_area52_followup_proc'),
(20045,'spell_area52_followup_proc'),
(851561,'spell_area52_followup_proc'),
(44400,'spell_area52_followup_proc'),
(44402,'spell_area52_followup_proc'),
(44403,'spell_area52_followup_proc'),
(760191,'spell_area52_followup_proc'),
(19298,'spell_area52_followup_proc');
DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_flame_throwing' AND `spell_id`=812352;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (812352,'spell_area52_flame_throwing');
DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_shared_might' AND `spell_id`=920043;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (920043,'spell_area52_shared_might');
