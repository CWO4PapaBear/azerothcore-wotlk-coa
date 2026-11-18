INSERT INTO spell_script_names (spell_id, ScriptName)
SELECT ranks.spell_id, 'spell_area52_dark_ranger_shadow_arrow'
FROM (
    SELECT 997527 AS spell_id
    UNION ALL SELECT 997528
    UNION ALL SELECT 997529
    UNION ALL SELECT 997530
    UNION ALL SELECT 997531
    UNION ALL SELECT 997532
    UNION ALL SELECT 997533
    UNION ALL SELECT 997534
    UNION ALL SELECT 997535
) AS ranks
WHERE NOT EXISTS (
    SELECT 1 FROM spell_script_names existing
    WHERE existing.spell_id = ranks.spell_id AND existing.ScriptName = 'spell_area52_dark_ranger_shadow_arrow'
);
