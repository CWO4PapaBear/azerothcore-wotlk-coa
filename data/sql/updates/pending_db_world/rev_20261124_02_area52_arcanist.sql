DELETE FROM `spell_script_names` WHERE `spell_id` IN (984117) AND `ScriptName` = 'aura_area52_arcanist_equipment';
DELETE FROM `spell_script_names` WHERE `spell_id` IN (984138) AND `ScriptName` = 'aura_area52_arcanist_potential';
DELETE FROM `spell_script_names` WHERE `spell_id` IN (984406,984407,984408,984409) AND `ScriptName` = 'aura_area52_arcanist_resonance';
DELETE FROM `spell_script_names` WHERE `spell_id` IN (984118,984122,984126,984130) AND `ScriptName` = 'spell_area52_arcanist_meteor';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(984117, 'aura_area52_arcanist_equipment'),
(984138, 'aura_area52_arcanist_potential'),
(984406, 'aura_area52_arcanist_resonance'),
(984407, 'aura_area52_arcanist_resonance'),
(984408, 'aura_area52_arcanist_resonance'),
(984409, 'aura_area52_arcanist_resonance'),
(984118, 'spell_area52_arcanist_meteor'),
(984122, 'spell_area52_arcanist_meteor'),
(984126, 'spell_area52_arcanist_meteor'),
(984130, 'spell_area52_arcanist_meteor');
