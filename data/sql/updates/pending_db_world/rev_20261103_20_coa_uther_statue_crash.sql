-- Upstream PR11010: retain the quest target while avoiding the broken HD collision model.
UPDATE `gameobject_template` SET `displayId` = 0 WHERE `entry` = 181653;
