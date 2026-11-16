DELETE FROM `creature` WHERE `guid` BETWEEN 9780140 AND 9780142;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`phaseMask`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`Comment`) VALUES
(9780140,967182,0,1519,1617,1,-8921.004369,486.342239,93.875700,2.218010,120,'Area52 temporary Tank dummy acceptance test'),
(9780141,666935,0,1519,1617,1,-8980.895393,559.968246,93.883500,5.536320,120,'Area52 temporary Healing dummy acceptance test'),
(9780142,666925,0,1519,1617,1,-8909.835631,494.783761,93.875700,2.218010,120,'Area52 temporary Execute dummy acceptance test');
