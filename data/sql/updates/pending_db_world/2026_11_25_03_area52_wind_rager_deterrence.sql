INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
SELECT 19263, 'spell_hun_area52_deterrence'
WHERE NOT EXISTS (SELECT 1 FROM `spell_script_names` WHERE `spell_id` = 19263 AND `ScriptName` = 'spell_hun_area52_deterrence');
