START TRANSACTION;

INSERT INTO spell_script_names (spell_id,ScriptName) VALUES
(954516,'spell_area52_blood_horror'),(954562,'spell_area52_blood_horror'),
(954564,'spell_area52_blood_horror'),(954566,'spell_area52_blood_horror');
INSERT INTO spell_proc (SpellId,ProcFlags,SpellTypeMask,SpellPhaseMask,AttributesMask,DisableEffectsMask,Chance) VALUES
(954516,1048576,1,2,2,4,100),(954562,1048576,1,2,2,4,100),
(954564,1048576,1,2,2,4,100),(954566,1048576,1,2,2,4,100);



INSERT INTO spell_proc (SpellId,SpellFamilyName,SpellFamilyMask0,SpellPhaseMask)
VALUES (863108,5,2,2);
INSERT INTO spell_script_names (spell_id,ScriptName)
VALUES (863108,'spell_warl_siphon_life');


DELETE FROM spell_script_names WHERE
    (spell_id IN (47569,47570) AND ScriptName = 'spell_pri_improved_shadowform')
    OR (spell_id = -47569 AND ScriptName = 'spell_pri_imp_shadowform');

COMMIT;
