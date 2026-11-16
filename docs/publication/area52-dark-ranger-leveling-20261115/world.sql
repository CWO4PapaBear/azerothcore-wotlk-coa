START TRANSACTION;
INSERT INTO spell_pet_auras (spell, effectId, pet, aura) VALUES (86395, 0, 0, 886395);

UPDATE spell_proc SET Chance=20 WHERE SpellId=81279 AND Chance=10 AND ProcFlags=64 AND SpellTypeMask=1 AND SpellPhaseMask=2 AND HitMask=3;

INSERT INTO spell_script_names (spell_id,ScriptName)
SELECT -982744,'spell_area52_dark_ranger_auto_shot' FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM spell_script_names WHERE spell_id=-982744 AND ScriptName='spell_area52_dark_ranger_auto_shot');

COMMIT;
