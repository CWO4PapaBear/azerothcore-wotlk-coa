START TRANSACTION;
UPDATE gameobject_template SET Data1=0, ScriptName='go_coa_cain_cellar_door'
WHERE entry=2300524 AND Data1=1903 AND ScriptName='';
UPDATE gameobject_template_addon SET flags=32 WHERE entry=2300524 AND flags=2;
COMMIT;
