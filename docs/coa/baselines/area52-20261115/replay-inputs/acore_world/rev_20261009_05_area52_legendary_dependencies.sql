DELETE FROM `spell_proc` WHERE `SpellId` IN (-701463, -48393, -48392, -48389, -44545, -44543, -44448, -44446, -44445, -18095, -18094, -17941, -12575, -12574, -12536, -11213, 11213, 12536, 12574, 12575, 17941, 18094, 18095, 44445, 44446, 44448, 44543, 44545, 48389, 48392, 48393, 701463);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(11213, 0, 0, 0, 0, 0, 87376, 1, 2, 0, 0, 0, 0, 5, 0, 0),
(12574, 0, 0, 0, 0, 0, 87376, 1, 2, 0, 0, 0, 0, 10, 0, 0),
(12575, 0, 0, 0, 0, 0, 87376, 1, 2, 0, 0, 0, 0, 15, 0, 0),
(18094, 0, 0, 0, 0, 0, 327680, 1, 2, 0, 0, 0, 0, 4, 5000, 0),
(18095, 0, 0, 0, 0, 0, 327680, 1, 2, 0, 0, 0, 0, 8, 5000, 0),
(44445, 0, 0, 0, 0, 0, 65536, 1, 2, 3, 0, 0, 0, 100, 0, 0),
(44446, 0, 0, 0, 0, 0, 65536, 1, 2, 3, 0, 0, 0, 100, 0, 0),
(44448, 0, 0, 0, 0, 0, 65536, 1, 2, 3, 0, 0, 0, 100, 0, 0),
(44543, 16, 0, 0, 0, 0, 65536, 1, 2, 0, 0, 6, 0, 7, 0, 0),
(44545, 16, 0, 0, 0, 0, 65536, 1, 2, 0, 0, 6, 0, 15, 0, 0),
(48389, 0, 0, 0, 0, 0, 139944, 1, 2, 0, 0, 0, 0, 8, 0, 0),
(48392, 0, 0, 0, 0, 0, 131752, 1, 2, 0, 0, 0, 0, 16, 0, 0),
(48393, 0, 0, 0, 0, 0, 131752, 1, 2, 0, 0, 0, 0, 24, 0, 0),
(12536, 0, 0, 0, 0, 0, 87312, 0, 1, 0, 12, 0, 0, 100, 0, 1),
(17941, 0, 5, 1, 16, 0, 65536, 1, 1, 0, 8, 0, 0, 100, 0, 1),
(701463, 0, 0, 0, 0, 0, 40, 1, 2, 1027, 0, 5, 0, 30, 2000, 0);
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_dependency_proc';
DELETE FROM `spell_script_names` WHERE `spell_id` IN (-18094, 18094, 18095) AND `ScriptName` = 'spell_warl_nightfall';
DELETE FROM `spell_script_names` WHERE `spell_id` IN (-44445, 44445, 44446, 44448) AND `ScriptName` = 'spell_mage_hot_streak';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(18094, 'spell_area52_dependency_proc'),
(18095, 'spell_area52_dependency_proc'),
(44445, 'spell_area52_dependency_proc'),
(44446, 'spell_area52_dependency_proc'),
(44448, 'spell_area52_dependency_proc'),
(44543, 'spell_area52_dependency_proc'),
(44545, 'spell_area52_dependency_proc'),
(48389, 'spell_area52_dependency_proc'),
(48392, 'spell_area52_dependency_proc'),
(48393, 'spell_area52_dependency_proc');
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_deep_freeze';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (44572, 'spell_area52_deep_freeze');
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_mana_barrier', 'spell_area52_siegebreaker', 'aura_area52_siegebreaker_collision');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (701463, 'spell_area52_mana_barrier'), (-954876, 'spell_area52_siegebreaker'), (954877, 'aura_area52_siegebreaker_collision');
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_missing_mana_surge';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (277432, 'aura_area52_missing_mana_surge');
DELETE FROM `spell_ranks` WHERE `first_spell_id` IN (33876, 33878) OR `spell_id` IN (33876, 33878, 33884, 33885, 33982, 33983, 33986, 33987, 48563, 48564, 48565, 48566, 302027, 302028, 302029, 302030, 302031, 302032, 302033, 302034);
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(33876, 33876, 1),
(33876, 302027, 2),
(33876, 302028, 3),
(33876, 302029, 4),
(33876, 302030, 5),
(33876, 33982, 6),
(33876, 33983, 7),
(33876, 48565, 8),
(33876, 48566, 9),
(33878, 33878, 1),
(33878, 302031, 2),
(33878, 302032, 3),
(33878, 302033, 4),
(33878, 302034, 5),
(33878, 33986, 6),
(33878, 33987, 7),
(33878, 48563, 8),
(33878, 48564, 9);
