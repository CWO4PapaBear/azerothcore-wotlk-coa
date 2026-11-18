-- Reviewed extra spawn, retained Area 52 spawn, shared creature entry.
CREATE TEMPORARY TABLE a52_spawn_pairs (extra INT PRIMARY KEY, retained INT, entry INT);
INSERT INTO a52_spawn_pairs VALUES
    (7500228, 9013030, 721111),
    (7500229, 9003110, 50340),
    (7500230, 9003102, 50325),
    (7500231, 9003106, 50292),
    (7500233, 9003104, 50324),
    (7500234, 9003115, 50291),
    (7500235, 9003105, 50280),
    (7500237, 9013028, 501266),
    (7500238, 9003107, 503410),
    (7500239, 9013022, 161844),
    (7500240, 9001000, 161701),
    (7500241, 9013029, 522923),
    (7500242, 9003109, 502923),
    (7500243, 9003103, 502770),
    (7500244, 9003114, 50289),
    (7500245, 9003111, 50283),
    (7500250, 9013017, 161710),
    (7500251, 9001001, 161700),
    (7500252, 9003100, 50295),
    (7500253, 9003113, 50287),
    (7500259, 9003101, 502960),
    (7500277, 9013000, 537),
    (7500279, 9013002, 537),
    (7500281, 9013004, 537),
    (7500283, 9013006, 537),
    (7500284, 9013007, 537),
    (7500285, 9013008, 537),
    (7500286, 9013009, 537),
    (7500287, 9013010, 537),
    (7500288, 9013011, 537),
    (7500289, 9013012, 537),
    (7500292, 9013015, 537),
    (7500294, 9003112, 50286),
    (7500306, 9013018, 161841),
    (7500307, 9013019, 161841),
    (7500308, 9013020, 161841),
    (7500309, 9013021, 161842),
    (7500315, 9001046, 161705),
    (7500316, 9001045, 161702),
    (7500344, 9003108, 50282),
    (7500352, 9001002, 161712),
    (7500372, 9013027, 399223),
    (9000139, 9013016, 537),
    (9074115, 9890212, 9780200),
    (9074135, 15980155, 9780200),
    (9970180, 9950081, 10558);
START TRANSACTION;
DELETE m FROM a52_spawn_pairs m
LEFT JOIN creature c ON c.guid=m.extra AND c.id=m.entry
LEFT JOIN creature k ON k.guid=m.retained AND k.id=m.entry AND k.map=c.map
WHERE c.guid IS NULL OR k.guid IS NULL;

-- Preserve Sister Alma's dialogue when quest 1660002 is rewarded.
UPDATE smart_scripts s JOIN a52_spawn_pairs m ON m.extra=7500316
SET s.entryorguid=-m.retained
WHERE s.entryorguid=-7500316 AND s.source_type=0 AND s.event_type=20 AND s.event_param1=1660002;
DELETE s FROM smart_scripts s JOIN a52_spawn_pairs m ON s.entryorguid=-m.extra
WHERE s.source_type=0 AND m.extra IN (7500251,7500285,7500240,7500315);
DELETE a FROM creature_addon a JOIN a52_spawn_pairs m ON a.guid=m.extra;
DELETE e FROM game_event_creature e JOIN a52_spawn_pairs m ON e.guid=m.extra;
DELETE e FROM custom_emote_marks e JOIN a52_spawn_pairs m ON e.guid=m.extra;
DELETE c FROM creature c JOIN a52_spawn_pairs m ON c.guid=m.extra;
COMMIT;
DROP TEMPORARY TABLE a52_spawn_pairs;
