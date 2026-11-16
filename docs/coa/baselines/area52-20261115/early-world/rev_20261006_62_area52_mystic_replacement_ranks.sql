DELETE FROM `spell_ranks` WHERE `first_spell_id` IN (983703, 926574, 954082, 978533, 982250);
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(983703, 983703, 1),
(983703, 983704, 2),
(983703, 983705, 3),
(983703, 983706, 4),
(983703, 983707, 5),
(926574, 926574, 1),
(926574, 926576, 2),
(926574, 926578, 3),
(926574, 926580, 4),
(926574, 926582, 5),
(926574, 926584, 6),
(926574, 926586, 7),
(926574, 926588, 8),
(954082, 954082, 1),
(954082, 954443, 2),
(954082, 954444, 3),
(954082, 954445, 4),
(954082, 954446, 5),
(954082, 954447, 6),
(954082, 954448, 7),
(954082, 954449, 8),
(954082, 954450, 9),
(978533, 978533, 1),
(978533, 978901, 2),
(978533, 978902, 3),
(978533, 978903, 4),
(978533, 978904, 5),
(978533, 978905, 6),
(978533, 978906, 7),
(978533, 978907, 8),
(982250, 982250, 1),
(982250, 300804, 2),
(982250, 300805, 3),
(982250, 300806, 4),
(982250, 300807, 5),
(982250, 300808, 6),
(982250, 300809, 7),
(982250, 300810, 8),
(982250, 300811, 9),
(982250, 300812, 10),
(982250, 300813, 11),
(982250, 300814, 12),
(982250, 300815, 13);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (978533, -978533) AND `ScriptName` IN ('spell_area52_hoplite_shield_strike', 'spell_area52_hoplite_requirements');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-978533, 'spell_area52_hoplite_shield_strike'),
(-978533, 'spell_area52_hoplite_requirements');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (982250, -982250) AND `ScriptName` IN ('spell_area52_hoplite_javelin', 'spell_area52_hoplite_requirements');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-982250, 'spell_area52_hoplite_javelin'),
(-982250, 'spell_area52_hoplite_requirements');
