START TRANSACTION;
INSERT INTO creature_template (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`)
SELECT 375036, 0, 0, 0, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, 0, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, 0, 0, 0, `PetSpellDataId`, `VehicleId`, 0, 0, '', `MovementType`, `HoverHeight`, 0.275, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, '', `VerifiedBuild`
FROM creature_template WHERE entry=11197;
INSERT INTO creature_template_model (CreatureID,Idx,CreatureDisplayID,DisplayScale,Probability,VerifiedBuild) SELECT 375036,Idx,CreatureDisplayID,DisplayScale,Probability,VerifiedBuild FROM creature_template_model WHERE CreatureID=11197;
INSERT INTO spell_ranks (first_spell_id,spell_id,`rank`) VALUES
(982331, 982331, 1),
(982331, 982332, 2),
(982331, 982333, 3),
(982331, 982334, 4),
(982331, 982335, 5),
(982331, 982336, 6),
(982331, 982337, 7),
(982331, 982338, 8),
(982331, 982339, 9),
(982331, 982340, 10),
(982331, 982341, 11),
(982331, 982342, 12),
(982331, 982343, 13),
(982744, 982744, 1),
(982744, 983170, 2),
(982744, 983171, 3),
(982744, 983172, 4),
(982744, 983173, 5),
(982744, 983174, 6),
(982744, 983175, 7),
(982744, 983176, 8),
(982744, 983177, 9);
INSERT INTO spell_proc (SpellId,SchoolMask,ProcFlags,SpellTypeMask,SpellPhaseMask,HitMask,Chance) VALUES (81279,0,64,1,2,3,10),(982743,1,320,1,2,3,33);
INSERT INTO spell_script_names (spell_id,ScriptName) VALUES (9931034,'aura_area52_dark_arrow_mark'),(982743,'aura_area52_bone_arrow_charges'),(-982744,'spell_area52_bone_arrow'),(997517,'spell_area52_dark_ranger_auto_shot'),(997526,'spell_area52_dark_ranger_auto_shot');
COMMIT;

