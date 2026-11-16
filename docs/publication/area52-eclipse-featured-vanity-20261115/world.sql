START TRANSACTION;
INSERT INTO spell_script_names (spell_id, ScriptName) VALUES
(81214, 'aura_area52_new_moon'),
(983848, 'spell_area52_waxing_moon_explosion');

INSERT INTO spell_proc
(SpellId, SchoolMask, SpellFamilyName, SpellFamilyMask0, SpellFamilyMask1, SpellFamilyMask2,
 ProcFlags, SpellTypeMask, SpellPhaseMask, HitMask, AttributesMask, DisableEffectsMask,
 ProcsPerMinute, Chance, Cooldown, Charges)
VALUES (81214, 0, 0, 0, 0, 0, 0x51010 | 0x100, 1, 2, 0, 0, 2, 0, 100, 0, 0);

INSERT INTO spell_script_names (spell_id, ScriptName) VALUES
(965282, 'aura_area52_total_eclipse_bonus'),
(48517, 'aura_area52_total_eclipse_bonus'),
(48518, 'aura_area52_total_eclipse_bonus');

INSERT INTO spell_linked_spell (spell_trigger, spell_effect, type, comment) VALUES
(977832, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977834, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977835, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977836, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977837, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977838, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977839, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977840, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977841, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977842, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977843, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977844, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977845, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability'),
(977846, 977833, 2, 'Area 52 Sunfire - main target spell vulnerability');

COMMIT;
