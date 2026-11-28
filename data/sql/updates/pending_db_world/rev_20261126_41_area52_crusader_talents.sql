DELETE FROM `spell_script_names` WHERE `spell_id` = 853512 AND `ScriptName` = 'aura_area52_power_of_light';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(853512, 'aura_area52_power_of_light');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (9452, 26016) AND `ScriptName` = 'aura_area52_vindication';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(9452, 'aura_area52_vindication'),
(26016, 'aura_area52_vindication');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (53379, 53484, 53648)
    AND `ScriptName` = 'aura_area52_swift_retribution_party';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(53379, 'aura_area52_swift_retribution_party'),
(53484, 'aura_area52_swift_retribution_party'),
(53648, 'aura_area52_swift_retribution_party');

INSERT IGNORE INTO `spell_group` (`id`, `spell_id`) VALUES
(2000182, 853379),
(2000182, 853484),
(2000182, 853648);
