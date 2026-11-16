UPDATE `spell_proc` SET `ProcFlags` = 68
WHERE `SpellId` IN (-13165, -61846) AND `ProcFlags` = 64
  AND `SpellFamilyName` = 0 AND `SpellTypeMask` = 0 AND `SpellPhaseMask` = 2 AND `HitMask` = 0;
