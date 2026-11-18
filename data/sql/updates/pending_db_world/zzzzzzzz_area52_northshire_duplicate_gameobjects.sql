-- Retain the Area 52 object, including the hand-placed stairwell door and event 73 bell.
CREATE TEMPORARY TABLE a52_object_pairs (extra INT PRIMARY KEY, retained INT, entry INT);
INSERT INTO a52_object_pairs VALUES
    (7500074, 7918024, 244614),
    (7500075, 7918000, 1617),
    (7500076, 7918001, 1617),
    (7500077, 7918002, 1617),
    (7500078, 7918003, 1617),
    (7500079, 7918004, 1617),
    (7500080, 7918005, 1617),
    (7500084, 7918016, 2061),
    (7500087, 7918013, 1731),
    (7500088, 7918014, 1731),
    (7500089, 7918015, 1731),
    (7500091, 7918006, 1618),
    (7500092, 7918007, 1618),
    (7500093, 7918008, 1618),
    (7500094, 7918009, 1618),
    (7500095, 7918010, 1618),
    (7500096, 7918011, 1618),
    (7500097, 7918012, 1618),
    (7500099, 26743, 176573),
    (7500100, 7918017, 22773),
    (7500131, 7910002, 2300501),
    (7500135, 7918020, 174849),
    (7500136, 7918030, 1600935),
    (7500137, 7918029, 1600933),
    (7500138, 7918028, 1600928),
    (7500156, 7910010, 2300523),
    (7500157, 7918025, 250002),
    (7500158, 7918026, 250002),
    (7500159, 7918021, 178646),
    (7500160, 7918022, 178646),
    (7500161, 7918023, 178646),
    (7500162, 7918031, 2300136),
    (7500163, 7918027, 515417),
    (7500164, 7910009, 2300522),
    (7500173, 7918019, 96383),
    (7500181, 30664, 161557);
START TRANSACTION;
DELETE g FROM gameobject g
JOIN a52_object_pairs p ON p.extra=g.guid AND p.entry=g.id
JOIN gameobject k ON k.guid=p.retained AND k.id=g.id AND k.map=g.map
WHERE (g.phaseMask & k.phaseMask) <> 0 AND (g.spawnMask & k.spawnMask) <> 0
  AND POW(g.position_x-k.position_x,2)+POW(g.position_y-k.position_y,2)+POW(g.position_z-k.position_z,2)
      <= IF(p.extra=7500131,0.005*0.005,0.00001*0.00001);
COMMIT;
DROP TEMPORARY TABLE a52_object_pairs;
