INSERT INTO spell_script_names (spell_id, ScriptName)
SELECT 998204, 'spell_area52_dark_ranger_artillery'
WHERE NOT EXISTS (
    SELECT 1 FROM spell_script_names
    WHERE spell_id=998204 AND ScriptName='spell_area52_dark_ranger_artillery'
);

INSERT INTO spell_script_names (spell_id, ScriptName)
SELECT 997526, 'spell_area52_dark_ranger_shadow_arrow'
WHERE NOT EXISTS (
    SELECT 1 FROM spell_script_names
    WHERE spell_id=997526 AND ScriptName='spell_area52_dark_ranger_shadow_arrow'
);

INSERT INTO spell_script_names (spell_id, ScriptName)
SELECT 984510, 'spell_area52_darkfall_periodic'
WHERE NOT EXISTS (SELECT 1 FROM spell_script_names WHERE spell_id=984510 AND ScriptName='spell_area52_darkfall_periodic');

DELETE FROM npc_vendor
WHERE entry=9781051 AND item BETWEEN 1339058 AND 1339064 AND slot=78+(item-1339058)
  AND maxcount=0 AND incrtime=0 AND ExtendedCost=900052 AND VerifiedBuild IS NULL;
