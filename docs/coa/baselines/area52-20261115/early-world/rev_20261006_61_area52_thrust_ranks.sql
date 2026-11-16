DELETE FROM `spell_ranks` WHERE `first_spell_id` = 978569;
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(978569, 978569, 1),
(978569, 978908, 2),
(978569, 978909, 3),
(978569, 978910, 4),
(978569, 978911, 5),
(978569, 978912, 6),
(978569, 978913, 7),
(978569, 978914, 8),
(978569, 978915, 9),
(978569, 978916, 10),
(978569, 978917, 11),
(978569, 978918, 12);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_hoplite_requirements', 'spell_area52_hoplite_thrust') AND `spell_id` IN (978569, -978569);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-978569, 'spell_area52_hoplite_requirements'),
(-978569, 'spell_area52_hoplite_thrust');
