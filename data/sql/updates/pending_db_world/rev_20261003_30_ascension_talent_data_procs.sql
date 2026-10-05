DELETE FROM `spell_script_names` WHERE (`spell_id`, `ScriptName`) IN (
    (-100, 'spell_warr_charge'),
    (12328, 'spell_warr_sweeping_strikes'),
    (-31656, 'spell_mage_empowered_fire'),
    (53563, 'spell_pal_beacon_of_light'),
    (-31244, 'spell_rog_quick_recovery'),
    (-48516, 'spell_dru_eclipse'),
    (-48539, 'spell_dru_revitalize'));

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('aura_ascension_eclipse', 'aura_ascension_revitalize');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-48516, 'aura_ascension_eclipse'),
(-48539, 'aura_ascension_revitalize');

UPDATE `spell_proc` SET `SpellFamilyName` = 0, `ProcFlags` = 0x10, `SpellTypeMask` = 1, `SpellPhaseMask` = 2,
    `AttributesMask` = 0
WHERE `SpellId` = 12328;

UPDATE `spell_proc` SET `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellTypeMask` = 1, `SpellPhaseMask` = 2,
    `HitMask` = 0
WHERE `SpellId` = -48516;

UPDATE `spell_proc` SET `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `ProcFlags` = 0x44000,
    `SpellTypeMask` = 2, `SpellPhaseMask` = 2, `Chance` = 100
WHERE `SpellId` = -48539;

DELETE FROM `spell_proc` WHERE `SpellId` = -31244;
