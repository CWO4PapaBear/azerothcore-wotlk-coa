DELETE FROM `creature_formations` WHERE `memberGUID` BETWEEN 9780130 AND 9780135;
DELETE FROM `creature` WHERE `guid` BETWEEN 9780130 AND 9780135;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `Comment`) VALUES
(9780130, 967254, 0, 1519, 1617, 1, -8961.530458, 571.275655, 93.83521, 5.4185076, 120, 'Area52 temporary AoE acceptance test'),
(9780131, 967254, 0, 1519, 1617, 1, -8963.589115, 572.148542, 93.83521, 5.4185076, 120, 'Area52 temporary AoE acceptance test'),
(9780132, 967254, 0, 1519, 1617, 1, -8962.067343, 573.446313, 93.83521, 5.4185076, 120, 'Area52 temporary AoE acceptance test'),
(9780133, 967254, 0, 1519, 1617, 1, -8965.647772, 573.021429, 93.83521, 5.4185076, 120, 'Area52 temporary AoE acceptance test'),
(9780134, 967254, 0, 1519, 1617, 1, -8964.126000, 574.319200, 93.83521, 5.4185076, 120, 'Area52 temporary AoE acceptance test'),
(9780135, 967254, 0, 1519, 1617, 1, -8962.604228, 575.616971, 93.83521, 5.4185076, 120, 'Area52 temporary AoE acceptance test');

INSERT INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`, `point_1`, `point_2`) VALUES
(9780130, 9780130, 0, 0, 0, 0, 0),
(9780130, 9780131, 0, 0, 0, 0, 0),
(9780130, 9780132, 0, 0, 0, 0, 0),
(9780130, 9780133, 0, 0, 0, 0, 0),
(9780130, 9780134, 0, 0, 0, 0, 0),
(9780130, 9780135, 0, 0, 0, 0, 0);
