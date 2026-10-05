-- The specialization spells swap a free-pick Hero's build, as they do for CoA classes and Wildcard Heroes.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_ascension_freepick_specialization_swap';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(979993, 'spell_ascension_freepick_specialization_swap'),
(979994, 'spell_ascension_freepick_specialization_swap'),
(979995, 'spell_ascension_freepick_specialization_swap'),
(979996, 'spell_ascension_freepick_specialization_swap'),
(979997, 'spell_ascension_freepick_specialization_swap'),
(979986, 'spell_ascension_freepick_specialization_swap'),
(979987, 'spell_ascension_freepick_specialization_swap'),
(979988, 'spell_ascension_freepick_specialization_swap');
