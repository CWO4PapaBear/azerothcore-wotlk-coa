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

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_hoplite_thrust', 'spell_area52_hoplite_bloodrage', 'aura_area52_hoplite_phalanx_damage', 'spell_area52_hoplite_javelin', 'aura_area52_hoplite_impaled', 'spell_area52_hoplite_colossus', 'spell_area52_hoplite_flurry');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(978569, 'spell_area52_hoplite_thrust'),
(2687, 'spell_area52_hoplite_bloodrage'),
(978539, 'aura_area52_hoplite_phalanx_damage'),
(978537, 'aura_area52_hoplite_phalanx_damage'),
(982250, 'spell_area52_hoplite_javelin'),
(982251, 'aura_area52_hoplite_impaled'),
(-86361, 'spell_area52_hoplite_colossus'),
(901200, 'spell_area52_hoplite_flurry');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_hoplite_incompatible';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(812295, 'aura_area52_hoplite_incompatible'),
(812676, 'aura_area52_hoplite_incompatible'),
(812677, 'aura_area52_hoplite_incompatible'),
(812300, 'aura_area52_hoplite_incompatible'),
(812959, 'aura_area52_hoplite_incompatible'),
(812960, 'aura_area52_hoplite_incompatible'),
(829623, 'aura_area52_hoplite_incompatible'),
(829634, 'aura_area52_hoplite_incompatible'),
(1584277, 'aura_area52_hoplite_incompatible'),
(954058, 'aura_area52_hoplite_incompatible'),
(277791, 'aura_area52_hoplite_incompatible');

DELETE FROM `spell_script_names` WHERE `spell_id` = 277595 AND `ScriptName` = 'spell_area52_hoplite_incompatible';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (277595, 'spell_area52_hoplite_incompatible');
