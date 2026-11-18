START TRANSACTION;
UPDATE creature_template SET name='Lesser Son of Flame',ScriptName='npc_son_of_flame_coa' WHERE entry IN (12143,112143,212143,312143) AND name='Son of Flame' AND ScriptName='' AND AIName='';
DELETE FROM `spell_script_names` WHERE `spell_id` = 805813 AND `ScriptName` = 'aura_ascension_barbarian_tankard';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(805813, 'aura_ascension_barbarian_tankard');

DELETE FROM `spell_script_names`
WHERE `spell_id` = -801669 AND `ScriptName` = 'spell_ascension_witch_doctor_dark_incantation';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-801669, 'spell_ascension_witch_doctor_dark_incantation');

DELETE FROM `spell_script_names` WHERE `spell_id` = 801530
AND `ScriptName` IN ('spell_ascension_necromancer_transfer_life', 'aura_ascension_necromancer_transfer_life');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(801530, 'aura_ascension_necromancer_transfer_life');
COMMIT;
