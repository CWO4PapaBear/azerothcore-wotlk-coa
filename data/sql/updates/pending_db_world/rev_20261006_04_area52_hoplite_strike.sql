DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_hoplite_shield_strike', 'aura_area52_hoplite_flurry', 'spell_area52_hoplite_flurry_damage');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(978533, 'spell_area52_hoplite_shield_strike'),
(300158, 'spell_area52_hoplite_shield_strike'),
(901200, 'aura_area52_hoplite_flurry'),
(901201, 'spell_area52_hoplite_flurry_damage');

DELETE FROM `spell_script_names` WHERE `spell_id` = 978217 AND `ScriptName` = 'aura_area52_hoplite_equipment';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (978217, 'aura_area52_hoplite_equipment');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_hoplite_requirements';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(978533, 'spell_area52_hoplite_requirements'),
(300158, 'spell_area52_hoplite_requirements'),
(978569, 'spell_area52_hoplite_requirements'),
(982250, 'spell_area52_hoplite_requirements'),
(901200, 'spell_area52_hoplite_requirements');
