DELETE FROM `spell_proc` WHERE `SpellId` = 984419;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(984419, 0, 7, 0, 0, 524288, 65536, 1, 1, 0, 0, 3, 0, 100, 0, 0);

UPDATE `spell_proc` SET `SpellFamilyMask2` = `SpellFamilyMask2` | 524288 WHERE `SpellId` = -48506;
