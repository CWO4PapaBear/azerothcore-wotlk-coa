START TRANSACTION;
UPDATE gameobject_template SET size=0.4
WHERE entry=254162 AND name='Eroded Sigil Stone' AND displayId=1025463 AND size=1;
INSERT INTO spell_script_names (spell_id,ScriptName) VALUES
(2105741,'spell_geddon_inferno_damage_coa'),
(2105742,'spell_geddon_inferno_damage_coa'),
(2105743,'spell_geddon_inferno_damage_coa'),
(2105744,'spell_geddon_inferno_damage_coa');
COMMIT;
