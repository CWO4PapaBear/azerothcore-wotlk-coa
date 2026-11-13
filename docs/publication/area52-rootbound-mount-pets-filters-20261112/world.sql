-- Original rideable mount. Separate entry from the tameable Wardrobe beast.
START TRANSACTION;
CREATE TEMPORARY TABLE rootbound_mount_guard (ok INT NOT NULL CHECK (ok=1));
INSERT INTO rootbound_mount_guard SELECT COUNT(*)=0 FROM creature_template WHERE entry=379987;
INSERT INTO rootbound_mount_guard SELECT COUNT(*)=0 FROM creature_template_model WHERE CreatureID=379987;
INSERT INTO rootbound_mount_guard SELECT COUNT(*)=1 FROM creature_template WHERE entry=915323 AND name='Emberback Plodder' AND family=0 AND type=12 AND AIName='' AND ScriptName='';
CREATE TEMPORARY TABLE rootbound_mount_template LIKE creature_template;
INSERT INTO rootbound_mount_template SELECT * FROM creature_template WHERE entry=915323;
UPDATE rootbound_mount_template SET entry=379987,name='Dark Rootbound Beast' WHERE entry=915323;
INSERT INTO creature_template SELECT * FROM rootbound_mount_template;
INSERT INTO creature_template_model (CreatureID,Idx,CreatureDisplayID,DisplayScale,Probability,VerifiedBuild)
VALUES (379987,0,142604,1,1,0);
INSERT INTO rootbound_mount_guard SELECT COUNT(*)=0 FROM creature_model_info WHERE DisplayID IN (48806,142604,1999412);
INSERT INTO creature_model_info (DisplayID,BoundingRadius,CombatReach,Gender,DisplayID_Other_Gender,VerifiedBuild) VALUES
(48806,0.347,1.5,2,0,0),(142604,0.347,1.5,2,0,0),(1999412,0.347,1.5,2,0,0);
COMMIT;
