UPDATE `spell_proc` SET `SpellFamilyMask1` = 1
WHERE `SpellId` = -53215 AND `SpellFamilyName` = 9
  AND `SpellFamilyMask0` = 1 AND `SpellFamilyMask1` = 0 AND `SpellFamilyMask2` = 0;

UPDATE `spell_proc` SET `SpellFamilyMask0` = 137216, `SpellFamilyMask1` = 8388609
WHERE `SpellId` = -53234 AND `SpellFamilyName` = 9
  AND `SpellFamilyMask0` = 131072 AND `SpellFamilyMask1` = 1 AND `SpellFamilyMask2` = 1;
