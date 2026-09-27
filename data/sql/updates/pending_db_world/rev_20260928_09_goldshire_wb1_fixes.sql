-- Goldshire WB1 follow-up: the rest area, spawns the new buildings swallowed, heights on the reshaped
-- terrain, the Midsummer site, Remy, guards and the Beginner's Books.
-- Geometry: patch-WB1.MPQ terrain (Azeroth_31/32_48-50.adt) and the collision triangles of its WMOs.

-- Inn rest area: the server and client AreaTrigger.dbc record 562 already follow the moved inn
UPDATE `areatrigger` SET `x` = -9483.25, `y` = -17.9883, `z` = 56.9634, `radius` = 30 WHERE `entry` = 562;

-- Creatures and eggs standing inside the new inn, stable, farmhouse and town hall
UPDATE `creature` SET `position_x` = -9467.53, `position_y` = -25.88, `position_z` = 57.249 WHERE `guid` = 80356 AND `id` = 883;
UPDATE `creature` SET `position_x` = -9468.51, `position_y` = -26.939, `position_z` = 57.18 WHERE `guid` = 80357 AND `id` = 890;
UPDATE `creature` SET `position_x` = -9492.08, `position_y` = -33.6, `position_z` = 56.731 WHERE `guid` = 81090 AND `id` = 883;
UPDATE `creature` SET `position_x` = -9493.23, `position_y` = -33.966, `position_z` = 56.776 WHERE `guid` = 81091 AND `id` = 890;
UPDATE `creature` SET `position_x` = -9440.041, `position_y` = -22.271, `position_z` = 59.435 WHERE `guid` = 80359 AND `id` = 721;
UPDATE `creature` SET `position_x` = -9558.771, `position_y` = 72.264, `position_z` = 58.969 WHERE `guid` = 80365 AND `id` = 721;
UPDATE `creature` SET `position_x` = -9576.585, `position_y` = -3.143, `position_z` = 62.647 WHERE `guid` = 80366 AND `id` = 1933;
UPDATE `creature` SET `position_x` = -9391.86, `position_y` = 49.981, `position_z` = 59.032 WHERE `guid` = 79648 AND `id` = 299;
UPDATE `creature` SET `position_x` = -9383.41, `position_y` = 59.619, `position_z` = 60.241 WHERE `guid` = 80342 AND `id` = 525;
UPDATE `creature` SET `position_x` = -9503.163, `position_y` = -26.857, `position_z` = 57.507 WHERE `guid` = 241320 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9491.5, `position_y` = -33.598, `position_z` = 56.793 WHERE `guid` = 241322 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9493.825, `position_y` = -33.623, `position_z` = 56.84 WHERE `guid` = 241323 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9467.189, `position_y` = -16.508, `position_z` = 57.139 WHERE `guid` = 241342 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9579.122, `position_y` = 35.624, `position_z` = 58.823 WHERE `guid` = 244224 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9546.146, `position_y` = 72.745, `position_z` = 58.93 WHERE `guid` = 241233 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9389.219, `position_y` = 52.122, `position_z` = 59.429 WHERE `guid` = 241458 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9382.783, `position_y` = 29.044, `position_z` = 59.655 WHERE `guid` = 243951 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9389.92, `position_y` = 28.023, `position_z` = 59.445 WHERE `guid` = 243960 AND `id` = 32820;
UPDATE `gameobject` SET `position_x` = -9427.385, `position_y` = 69.131, `position_z` = 56.784 WHERE `guid` = 151926 AND `id` = 113768;
UPDATE `gameobject` SET `position_x` = -9424.212, `position_y` = 53.985, `position_z` = 57.12 WHERE `guid` = 151927 AND `id` = 113769;

-- Spawns left floating or buried by the reshaped terrain
UPDATE `creature` SET `position_x` = -9455.02, `position_y` = -29.289, `position_z` = 57.334 WHERE `guid` = 79644 AND `id` = 721;
UPDATE `creature` SET `position_x` = -9499, `position_y` = -56, `position_z` = 57.643 WHERE `guid` = 241308 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9491, `position_y` = -53, `position_z` = 57.259 WHERE `guid` = 241319 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9375.14, `position_y` = 26.349, `position_z` = 59.744 WHERE `guid` = 244095 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9373.36, `position_y` = 22.896, `position_z` = 59.76 WHERE `guid` = 244097 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9382.33, `position_y` = 14.482, `position_z` = 59.274 WHERE `guid` = 244123 AND `id` = 32820;
UPDATE `creature` SET `position_x` = -9585.09, `position_y` = 22.494, `position_z` = 59.372 WHERE `guid` = 243747 AND `id` = 32820;

-- Midsummer Fire Festival site: the stock spot is inside the new farmhouse; the whole site moves 17 yd
-- south-east to open ground, keeping each piece's height above the terrain
UPDATE `creature` SET `position_x` = -9382.23, `position_y` = 9.152, `position_z` = 59.503 WHERE `guid` = 86250 AND `id` = 26401;
UPDATE `creature` SET `position_x` = -9375.87, `position_y` = 1.91, `position_z` = 61.316 WHERE `guid` = 86714 AND `id` = 25962;
UPDATE `creature` SET `position_x` = -9371.14, `position_y` = 9.349, `position_z` = 59.913 WHERE `guid` = 94559 AND `id` = 16781;
UPDATE `creature` SET `position_x` = -9379.17, `position_y` = -5.454, `position_z` = 62.758 WHERE `guid` = 94560 AND `id` = 16781;
UPDATE `creature` SET `position_x` = -9369.36, `position_y` = 5.896, `position_z` = 60.305 WHERE `guid` = 94561 AND `id` = 16781;
UPDATE `creature` SET `position_x` = -9378.33, `position_y` = -2.518, `position_z` = 62.511 WHERE `guid` = 94619 AND `id` = 16781;
UPDATE `creature` SET `position_x` = -9388.21, `position_y` = 22.502, `position_z` = 59.233 WHERE `guid` = 94768 AND `id` = 26258;
UPDATE `creature` SET `position_x` = -9372.28, `position_y` = -6.634, `position_z` = 70.527 WHERE `guid` = 94849 AND `id` = 17066;
UPDATE `creature` SET `position_x` = -9391.2, `position_y` = 19.633, `position_z` = 58.998 WHERE `guid` = 245555 AND `id` = 16592;
UPDATE `creature` SET `position_x` = -9385.17, `position_y` = 9.724, `position_z` = 59.174 WHERE `guid` = 245655 AND `id` = 25898;
UPDATE `gameobject` SET `position_x` = -9381.03, `position_y` = 13.138, `position_z` = 59.29 WHERE `guid` = 50792 AND `id` = 181302;
UPDATE `gameobject` SET `position_x` = -9378.95, `position_y` = 12.574, `position_z` = 59.357 WHERE `guid` = 50805 AND `id` = 181302;
UPDATE `gameobject` SET `position_x` = -9379.36, `position_y` = 6.516, `position_z` = 59.719 WHERE `guid` = 50822 AND `id` = 181305;
UPDATE `gameobject` SET `position_x` = -9380.07, `position_y` = 10.685, `position_z` = 59.31 WHERE `guid` = 50891 AND `id` = 181306;
UPDATE `gameobject` SET `position_x` = -9379.01, `position_y` = 5.887, `position_z` = 60.695 WHERE `guid` = 50933 AND `id` = 181307;
UPDATE `gameobject` SET `position_x` = -9372.28, `position_y` = -6.634, `position_z` = 63.944 WHERE `guid` = 51040 AND `id` = 181605;
UPDATE `gameobject` SET `position_x` = -9409.28, `position_y` = 19.496, `position_z` = 57.597 WHERE `guid` = 51591 AND `id` = 181355;
UPDATE `gameobject` SET `position_x` = -9388.81, `position_y` = 30.642, `position_z` = 59.449 WHERE `guid` = 51592 AND `id` = 181355;
UPDATE `gameobject` SET `position_x` = -9364.33, `position_y` = -0.743, `position_z` = 62.263 WHERE `guid` = 51593 AND `id` = 181355;
UPDATE `gameobject` SET `position_x` = -9386.52, `position_y` = -5.42, `position_z` = 61.055 WHERE `guid` = 51840 AND `id` = 181355;
UPDATE `gameobject` SET `position_x` = -9379.96, `position_y` = 19.723, `position_z` = 59.472 WHERE `guid` = 51841 AND `id` = 181355;
UPDATE `gameobject` SET `position_x` = -9368.2, `position_y` = 10.254, `position_z` = 60.009 WHERE `guid` = 51987 AND `id` = 181355;
UPDATE `gameobject` SET `position_x` = -9391.51, `position_y` = 0.471, `position_z` = 60.271 WHERE `guid` = 52349 AND `id` = 188020;
UPDATE `gameobject` SET `position_x` = -9365.81, `position_y` = 2.157, `position_z` = 61.313 WHERE `guid` = 52417 AND `id` = 188020;
UPDATE `gameobject` SET `position_x` = -9379.84, `position_y` = -11.403, `position_z` = 62.711 WHERE `guid` = 52418 AND `id` = 188020;
UPDATE `gameobject` SET `position_x` = -9404.72, `position_y` = 13.978, `position_z` = 58.106 WHERE `guid` = 52419 AND `id` = 188020;
UPDATE `gameobject` SET `position_x` = -9381.06, `position_y` = 9.93, `position_z` = 59.238 WHERE `guid` = 52516 AND `id` = 188021;
UPDATE `gameobject` SET `position_x` = -9390.21, `position_y` = 20.502, `position_z` = 58.666 WHERE `guid` = 76307 AND `id` = 187928;
UPDATE `gameobject` SET `position_x` = -9391.2, `position_y` = 19.633, `position_z` = 58.915 WHERE `guid` = 242594 AND `id` = 181371;

-- Darkmoon Faire (Elwynn): its tents and rides stand where the new town hall and keep ruins are; the faire
-- stays on its Mulgore and Terokkar rotation
UPDATE `game_event` SET `end_time` = '2000-01-01 00:00:00' WHERE `eventEntry` IN (4, 23);

-- Remy "Two Times" sells his produce beside the scarecrow and pumpkin patch of the new farm
UPDATE `creature` SET `position_x` = -9503, `position_y` = 70.5, `position_z` = 56.63, `orientation` = 3.14159 WHERE `guid` = 80326 AND `id` = 241;

-- Stormwind Guards at the new village gate and the town hall steps
DELETE FROM `creature` WHERE `guid` BETWEEN 9001000 AND 9001003;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`) VALUES
(9001000, 1423, 0, 0, 0, 1, 1, 1, -9399.5, -12.5, 62.228, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 guard'),
(9001001, 1423, 0, 0, 0, 1, 1, 1, -9399.5, -20.5, 62.234, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 guard'),
(9001002, 1423, 0, 0, 0, 1, 1, 1, -9522, 42.634, 58.598, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 guard'),
(9001003, 1423, 0, 0, 0, 1, 1, 1, -9521.5, 55.5, 58.997, 0, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 guard');

-- Beginner's Book of Ascension and Beginner's Book of Artisans at the market square
DELETE FROM `creature` WHERE `guid` BETWEEN 9001004 AND 9001005;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`) VALUES
(9001004, 75118, 0, 0, 0, 1, 1, 0, -9484.3, 37.6, 56.667, 3.14159, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 market book'),
(9001005, 57524, 0, 0, 0, 1, 1, 0, -9479.4, 33.5, 56.807, 3.14159, 300, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 0, 'Goldshire WB1 market book');
