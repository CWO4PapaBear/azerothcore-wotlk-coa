DELETE FROM `spell_proc` WHERE `SpellId` IN (-29834, 29834, 29838, 829838);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(29834, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 6, 0, 100, 30000, 0),
(29838, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 6, 0, 100, 30000, 0),
(829838, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 6, 0, 100, 30000, 0);
