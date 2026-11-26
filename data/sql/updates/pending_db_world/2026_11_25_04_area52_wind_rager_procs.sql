INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
SELECT 978232, 'aura_area52_wind_rager'
WHERE NOT EXISTS (
    SELECT 1 FROM `spell_script_names`
    WHERE `spell_id` = 978232 AND `ScriptName` = 'aura_area52_wind_rager'
);

INSERT INTO `spell_group` (`id`, `spell_id`)
SELECT 1101, 412004
WHERE NOT EXISTS (SELECT 1 FROM `spell_group` WHERE `id` = 1101 AND `spell_id` = 412004);

DELETE FROM `spell_proc` WHERE `SpellId` = 978232;
INSERT INTO `spell_proc`
(`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`,
 `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
 `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES (978232, 1, 0, 0, 0, 0, 0, 1, 2, 0, 0, 6, 0, 10, 6000, 0);

DELETE FROM `spell_proc` WHERE `SpellId` = 997084;
INSERT INTO `spell_proc`
(`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`,
 `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
 `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES (997084, 0, 11, 1, 0, 0, 65536, 1, 1, 0, 8, 0, 0, 100, 0, 1);
