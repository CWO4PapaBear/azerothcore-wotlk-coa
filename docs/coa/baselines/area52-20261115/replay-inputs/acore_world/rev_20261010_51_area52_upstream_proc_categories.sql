DELETE FROM `spell_proc` WHERE `SpellId` IN (275235, 272057, 978742, 84750, 84752);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
    `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`,
    `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(978742, 0, 8, 0x2, 0, 0, 0x10, 0x1, 0x2, 0, 0, 6, 0, 100, 0, 0),
(84750, 0, 0, 0, 0, 0, 0x50110, 0x1, 0x2, 0, 0, 0, 0, 100, 0, 0),
(84752, 0, 0, 0, 0, 0, 0x50110, 0x1, 0x2, 0, 0, 0, 0, 100, 0, 0),
(275235, 0, 8, 0x2, 0, 0, 0x10, 0x1, 0x2, 0, 0, 0, 0, 100, 0, 0),
(272057, 0, 3, 0x60, 0, 0, 0x10000, 0x1, 0x2, 0, 0, 0, 0, 100, 0, 0);
DELETE FROM `spell_group` WHERE `id` = 2106089;
INSERT INTO `spell_group` (`id`, `spell_id`) VALUES
(2106089, 11207),
(2106089, 14889),
(2106089, 16089),
(2106089, 16909),
(2106089, 17959),
(2106089, 33221),
(2106089, 35578),
(2106089, 44449),
(2106089, 61758);
DELETE FROM `spell_group_stack_rules` WHERE `group_id` = 2106089;
INSERT INTO `spell_group_stack_rules` (`group_id`, `stack_rule`, `description`) VALUES
(2106089, 3, 'Area 52 spell critical damage talents: highest bonus only');
DELETE FROM `spell_script_names` WHERE `spell_id` IN (84750, 84752) AND `ScriptName` = 'aura_wildcard_unique_physical_ability';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(84750, 'aura_wildcard_unique_physical_ability'),
(84752, 'aura_wildcard_unique_physical_ability');
