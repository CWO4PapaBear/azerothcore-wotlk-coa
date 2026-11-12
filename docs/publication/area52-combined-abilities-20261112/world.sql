START TRANSACTION;
INSERT INTO spell_script_names (spell_id, ScriptName)
SELECT 954583, 'aura_area52_frenzied_cleave' FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM spell_script_names WHERE spell_id = 954583 AND ScriptName = 'aura_area52_frenzied_cleave');

-- Negative rank-one ID binds the script to each rank through the native rank loader.
INSERT INTO spell_script_names (spell_id, ScriptName)
SELECT -760122, 'spell_area52_void_eruption' FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM spell_script_names WHERE spell_id = -760122 AND ScriptName = 'spell_area52_void_eruption');

-- All player ranks inherit this row. Count damage actually dealt, including
-- triggered explosions and partially absorbed or blocked hits.
UPDATE spell_proc SET ProcFlags = 1048576, AttributesMask = 2, HitMask = 1091
WHERE SpellId = -27243 AND ProcFlags = 0 AND AttributesMask = 0 AND HitMask = 0;
COMMIT;
