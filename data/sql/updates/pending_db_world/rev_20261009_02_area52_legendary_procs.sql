DELETE FROM `spell_proc` WHERE `SpellId` IN (84508, 84512, 965500, 965501, 965502);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(84508, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 33, 0, 0),
(84512, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 33, 0, 0),
(965500, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 35, 0, 0),
(965501, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 3000, 0),
(965502, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 40, 3000, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_legendary_proc', 'spell_area52_purification_hit', 'aura_area52_purification_ground', 'spell_area52_zealot_hit');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(84508, 'spell_area52_legendary_proc'),
(84512, 'spell_area52_legendary_proc'),
(965500, 'spell_area52_legendary_proc'),
(965501, 'spell_area52_legendary_proc'),
(965502, 'spell_area52_legendary_proc'),
(84509, 'spell_area52_purification_hit'),
(84759, 'aura_area52_purification_ground'),
(965508, 'spell_area52_zealot_hit');

DELETE FROM `spell_bonus_data` WHERE `entry` = 84760;
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(84760, 0.2, 0, 0.135, 0, 'Area 52 - Amal\'thazad\'s Curse');

DELETE FROM `spell_bonus_data` WHERE `entry` = 84509;
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`)
SELECT 84509, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, 'Area 52 - Purification Exorcism'
FROM `spell_bonus_data` WHERE `entry` = 879;
DELETE FROM `spell_bonus_data` WHERE `entry` = 84759;
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`)
SELECT 84759, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, 'Area 52 - Purification Consecration'
FROM `spell_bonus_data` WHERE `entry` = 26573;
