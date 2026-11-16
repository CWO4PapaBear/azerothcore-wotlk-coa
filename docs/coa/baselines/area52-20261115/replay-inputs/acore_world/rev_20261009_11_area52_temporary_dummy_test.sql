DELETE FROM `creature_formations` WHERE `memberGUID` BETWEEN 9780120 AND 9780125;
DELETE FROM `creature` WHERE `guid` BETWEEN 9780120 AND 9780125;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `Comment`) VALUES
(9780120, 967254, 0, 1519, 1617, 1, -8906.328935, 502.200083, 93.85713, 2.312756, 120, 'Area52 temporary AoE acceptance test'),
(9780121, 967254, 0, 1519, 1617, 1, -8904.240322, 501.401525, 93.85713, 2.312756, 120, 'Area52 temporary AoE acceptance test'),
(9780122, 967254, 0, 1519, 1617, 1, -8905.714613, 500.050058, 93.85713, 2.312756, 120, 'Area52 temporary AoE acceptance test'),
(9780123, 967254, 0, 1519, 1617, 1, -8902.151708, 500.602968, 93.85713, 2.312756, 120, 'Area52 temporary AoE acceptance test'),
(9780124, 967254, 0, 1519, 1617, 1, -8903.626000, 499.251500, 93.85713, 2.312756, 120, 'Area52 temporary AoE acceptance test'),
(9780125, 967254, 0, 1519, 1617, 1, -8905.100292, 497.900032, 93.85713, 2.312756, 120, 'Area52 temporary AoE acceptance test');

INSERT INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`, `point_1`, `point_2`) VALUES
(9780120, 9780120, 0, 0, 0, 0, 0),
(9780120, 9780121, 0, 0, 0, 0, 0),
(9780120, 9780122, 0, 0, 0, 0, 0),
(9780120, 9780123, 0, 0, 0, 0, 0),
(9780120, 9780124, 0, 0, 0, 0, 0),
(9780120, 9780125, 0, 0, 0, 0, 0);
