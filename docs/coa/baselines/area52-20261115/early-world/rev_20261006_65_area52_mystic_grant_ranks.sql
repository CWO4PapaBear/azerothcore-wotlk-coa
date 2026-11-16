DELETE FROM `spell_ranks` WHERE `first_spell_id` IN (92575, 92584) OR `spell_id` IN (92575, 93938, 93939, 93940, 93941, 93942, 93943, 93944, 92584, 94064, 94065, 94066, 94067, 94068, 94069, 94070);
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(92575, 92575, 1),
(92575, 93938, 2),
(92575, 93939, 3),
(92575, 93940, 4),
(92575, 93941, 5),
(92575, 93942, 6),
(92575, 93943, 7),
(92575, 93944, 8),
(92584, 92584, 1),
(92584, 94064, 2),
(92584, 94065, 3),
(92584, 94066, 4),
(92584, 94067, 5),
(92584, 94068, 6),
(92584, 94069, 7),
(92584, 94070, 8);

DELETE FROM `spell_bonus_data` WHERE `entry` IN (92575, 92584);
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(92575, 0.129, 0, 0.129, 0, 'Area 52 Carrion Swarm'),
(92584, 0, 0.06, 0, 0.06, 'Area 52 Venom Belch');
