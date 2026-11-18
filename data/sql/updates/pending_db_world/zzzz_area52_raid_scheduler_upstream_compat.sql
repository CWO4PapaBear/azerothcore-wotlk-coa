-- Preserve existing Area 52 rows when importing upstream scheduler support.
CREATE TABLE IF NOT EXISTS `coa_boss_summon` (
  `entry`             INT UNSIGNED NOT NULL COMMENT 'boss entry, coa_boss.entry',
  `idx`               INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'position within this boss, multiple summons',
  `summon_entry`      INT UNSIGNED NOT NULL DEFAULT 0,
  `summon_delay_ms`   INT UNSIGNED NOT NULL DEFAULT 0,
  `summon_buff_spell` INT UNSIGNED NOT NULL DEFAULT 0,
  `replace_entry`     INT UNSIGNED NOT NULL DEFAULT 0
    COMMENT 'despawn the nearest live creature of this entry near the boss and summon at its spot, 0 none',
  `replace_radius`    FLOAT NOT NULL DEFAULT 0 COMMENT 'search radius in yards for replace_entry',
  `comment`           VARCHAR(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`entry`, `idx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `coa_boss_summon` (`entry`, `idx`, `summon_entry`, `summon_delay_ms`, `summon_buff_spell`, `comment`) VALUES
(12118, 0, 12268, 5000, 2105223, 'Lucifron summons Shadow of Lucifron') ON DUPLICATE KEY UPDATE `entry` = `coa_boss_summon`.`entry`;

-- Full column list, difficulty variants matching every other MC add (Firesworn:
-- 12099/112099/212099/312099; Magmadar's heads: rev_20260930_91).
INSERT INTO `creature_template`
  (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`,
     `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`,
     `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`,
     `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`,
     `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`,
     `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`,
     `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`,
     `flags_extra`, `ScriptName`)
VALUES
(12268, 112268, 212268, 312268, 12268, 12268, 'Shadow of Lucifron', NULL, NULL, 0, 63, 63, 0, 54, 0, 1, 1.14286, 1,
 1, 20, 3, 0, 13, 2000, 2000, 1, 1, 2, 0, 0, 0, 0, 10, 72, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
 'npc_shadow_of_lucifron_coa'),
(112268, 0, 0, 0, 12268, 12268, 'Shadow of Lucifron', NULL, NULL, 0, 63, 63, 0, 54, 0, 1, 1.14286, 1, 1, 20, 3, 0,
 13, 2000, 2000, 1, 1, 2, 0, 0, 0, 0, 10, 72, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
 'npc_shadow_of_lucifron_coa'),
(212268, 0, 0, 0, 12268, 12268, 'Shadow of Lucifron', NULL, NULL, 0, 63, 63, 0, 54, 0, 1, 1.14286, 1, 1, 20, 3, 0,
 13, 2000, 2000, 1, 1, 2, 0, 0, 0, 0, 10, 72, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
 'npc_shadow_of_lucifron_coa'),
(312268, 0, 0, 0, 12268, 12268, 'Shadow of Lucifron', NULL, NULL, 0, 63, 63, 0, 54, 0, 1, 1.14286, 1, 1, 20, 3, 0,
 13, 2000, 2000, 1, 1, 2, 0, 0, 0, 0, 10, 72, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
 'npc_shadow_of_lucifron_coa')
ON DUPLICATE KEY UPDATE `entry` = `creature_template`.`entry`;

-- Model: export display_ids {13031} -- Lucifron's own display id (measured, not placeholder).
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(12268, 0, 13031, 1, 1),
(112268, 0, 13031, 1, 1),
(212268, 0, 13031, 1, 1),
(312268, 0, 13031, 1, 1) ON DUPLICATE KEY UPDATE `CreatureID` = `creature_template_model`.`CreatureID`;

-- Health: [designed] 25% of Lucifron's own per-player coa_boss_flex figures (rev_20260930_81);
-- this is an addition to the fight, not a split of Lucifron's own total, so his row is untouched.
INSERT INTO `coa_boss_flex` (`entry`, `hp_d0`, `hp_d1`, `hp_d2`, `hp_d3`, `comment`) VALUES
(12268, 107953, 143937, 216638, 315873, '[designed] 25% of Lucifron''s own flex figures') ON DUPLICATE KEY UPDATE `entry` = `coa_boss_flex`.`entry`;

-- Spell script: dummy-then-effect chain for Shadow Bolt, same idiom as the other MC adds'
-- damage-info chains (spell_magmadar_head_scorching_breath_tick, rev_20260930_91).
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(2105254, 'spell_shadow_of_lucifron_shadow_bolt') ON DUPLICATE KEY UPDATE `spell_id` = `spell_script_names`.`spell_id`;

-- rev_20261001_02 fixed the Arcane Force Nova's damage AMOUNT (coa_spell_damage_info bound
-- 2105617 to the real per-difficulty "Arcane Force Nova - Hidden Area Damage" base points
-- 6999/9333/11667/11999) but left `coa_boss_schedule`'s dummy (2105612) completing into 2105617
-- itself. Live reproduction (Ghost harness, grouped 5-bot raid, real Shazzrah spawn) confirms the
-- defect that fix did not touch: 2105617's own EffectImplicitTargetA is TARGET_UNIT_TARGET_ANY
-- (Spell.dbc), a single-unit hit on whichever target CoaBossAI::Cast() passed in (the tank) -- not
-- an area. One Nova landed exactly one real hit (7413 on bot01, the tank) and zero on the other
-- four raid members: the real damage number reached one player, never "the raid".
--
-- The actual area spells are 2105613-16 themselves (EffectImplicitTargetA
-- TARGET_UNIT_DEST_AREA_ENEMY, EffectRadiusIndex 12 ~10yd, SPELL_ATTR0_DO_NOT_DISPLAY +
-- SPELL_ATTR0_DO_NOT_LOG -- a Blizzard "hidden effect" meant to be triggered by a visible dummy,
-- never cast directly). No Spell.dbc row (2105612/2105617 included) carries SpellDifficultyId
-- 2114, the SpellDifficulty.dbc group whose four SpellID columns happen to equal 2105613-16: that
-- group is orphaned data, not a resolution path this engine can reach by casting any one id, so
-- CoaBossAI's `effect` column -- a single id meant to be shared across all four difficulties --
-- cannot name this family by itself, unlike every other dummy/effect pair in this table.
--
-- Fix at the root: let `effect` differ per difficulty, the same shape `spell_d0..d3` already uses
-- for the few casts whose id genuinely differs by tier. CoaBossAI.cpp (this revision) now resolves
-- the pending post-dummy cast the same way it resolves the dummy itself. Existing rows keep their
-- single `effect` value copied into all four new columns (no behaviour change for Lucifron,
-- Gehennas, Garr, Baron Geddon, Sulfuron and Golemagg's own dummy/effect pairs). Shazzrah's Nova
-- row alone gets the real per-difficulty hidden spells, and no longer needs the coa_spell_damage_info
-- read-the-amount-back retrofit: 2105613-16 already carry their own correct EffectBasePoints and
-- area targeting, so that row and its spell_script_names binding (both 2105617, now never cast) are
-- dropped as dead data.

SELECT IF(
    NOT EXISTS (SELECT 1 FROM `information_schema`.`COLUMNS`
        WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'coa_boss_schedule' AND `COLUMN_NAME` = 'effect_d0'),
    "ALTER TABLE `coa_boss_schedule` "
        "CHANGE COLUMN `effect` `effect_d0` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'cast when the dummy completes, 0 none', "
        "ADD COLUMN `effect_d1` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'cast when the dummy completes, 0 none' AFTER `effect_d0`, "
        "ADD COLUMN `effect_d2` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'cast when the dummy completes, 0 none' AFTER `effect_d1`, "
        "ADD COLUMN `effect_d3` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'cast when the dummy completes, 0 none' AFTER `effect_d2`",
    'DO 0') INTO @coa_boss_schedule_effect_split;
PREPARE `coa_boss_schedule_effect_split` FROM @coa_boss_schedule_effect_split;
EXECUTE `coa_boss_schedule_effect_split`;
DEALLOCATE PREPARE `coa_boss_schedule_effect_split`;

UPDATE `coa_boss_schedule` SET `effect_d1` = `effect_d0`, `effect_d2` = `effect_d0`, `effect_d3` = `effect_d0`
    WHERE `effect_d0` != 0 AND `effect_d1` = 0 AND `effect_d2` = 0 AND `effect_d3` = 0;

UPDATE `coa_boss_schedule` SET `effect_d0` = 2105613, `effect_d1` = 2105614, `effect_d2` = 2105615, `effect_d3` = 2105616
    WHERE `entry` = 12264 AND `idx` = 6 AND 2105617 IN (`effect_d0`) AND `effect_d1` = 2105617 AND `effect_d2` = 2105617 AND `effect_d3` = 2105617;

