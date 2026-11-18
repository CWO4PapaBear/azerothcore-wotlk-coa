-- Seed missing rows only: retain later endgame tuning and never multiply an existing endgame value again.
INSERT IGNORE INTO `manastorm_Endgame_scaling_curve`
  (`depth`, `health_multiplier`, `global_damage_multiplier`, `melee_multiplier`, `spell_multiplier`)
SELECT `depth`, `health_multiplier`, `global_damage_multiplier` * 2, `melee_multiplier`, `spell_multiplier`
FROM `manastorm_scaling_curve`;

INSERT IGNORE INTO `manastorm_Endgame_group_scaling`
  (`min_members`, `max_members`, `health_multiplier`, `global_damage_multiplier`, `melee_multiplier`, `spell_multiplier`, `enabled`)
SELECT `min_members`, `max_members`, `health_multiplier`, `global_damage_multiplier`, `melee_multiplier`, `spell_multiplier`, `enabled`
FROM `manastorm_group_scaling`;
