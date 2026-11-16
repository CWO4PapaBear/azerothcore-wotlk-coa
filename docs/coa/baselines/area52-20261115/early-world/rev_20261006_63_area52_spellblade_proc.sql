DELETE FROM `spell_proc` WHERE `SpellId` IN (812575, 812576, 812577);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(812575, 0, 0, 0, 0, 0, 20, 1, 2, 0, 0, 0, 0, 0, 0, 0),
(812576, 0, 0, 0, 0, 0, 20, 1, 2, 0, 0, 0, 0, 0, 0, 0),
(812577, 0, 0, 0, 0, 0, 20, 1, 2, 0, 0, 0, 0, 0, 0, 0);
