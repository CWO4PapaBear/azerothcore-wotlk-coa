UPDATE `creature_template` SET `rank` = 3
WHERE (`entry` IN (9816, 10264, 10363) AND `rank` = 1)
   OR (`entry` IN (10509, 10899) AND `rank` = 2);

UPDATE `creature_equip_template` SET `ItemID1` = 5196, `ItemID2` = 5196
WHERE `CreatureID` = 646 AND `ID` = 2 AND `ItemID1` = 2179 AND `ItemID2` = 2179;

UPDATE `gameobject_template` SET `ScriptName` = 'go_cache_of_the_firelord_coa'
WHERE `entry` = 179703 AND `ScriptName` = '' AND `type` = 3;
