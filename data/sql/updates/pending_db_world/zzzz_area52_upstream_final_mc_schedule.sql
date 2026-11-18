START TRANSACTION;

INSERT INTO `coa_boss_schedule` (`entry`, `idx`, `spell_d0`, `spell_d1`, `spell_d2`, `spell_d3`, `effect_d0`, `effect_d1`, `effect_d2`, `effect_d3`, `first_ms`, `first_max_ms`, `period_ms`, `period_max_ms`, `hp_pct`, `target`, `comment`)
SELECT 11988, 3, 2105802, 2105802, 2105802, 2105802, 0, 0, 0, 0, 10000, 0, 18000, 0, 0, 0, CONVERT(X'4d61676d612053706c617368205b64657369676e65645d' USING utf8mb4) WHERE NOT EXISTS (SELECT 1 FROM `coa_boss_schedule` WHERE `entry` = 11988 AND `idx` = 3);

INSERT INTO `coa_boss_schedule` (`entry`, `idx`, `spell_d0`, `spell_d1`, `spell_d2`, `spell_d3`, `effect_d0`, `effect_d1`, `effect_d2`, `effect_d3`, `first_ms`, `first_max_ms`, `period_ms`, `period_max_ms`, `hp_pct`, `target`, `comment`)
SELECT 11988, 4, 2105806, 2105806, 2105806, 2105806, 0, 0, 0, 0, 16000, 0, 32000, 0, 0, 0, CONVERT(X'4d6f6c74656e2041726d6f72205b64657369676e65645d' USING utf8mb4) WHERE NOT EXISTS (SELECT 1 FROM `coa_boss_schedule` WHERE `entry` = 11988 AND `idx` = 4);

INSERT INTO `coa_boss_schedule` (`entry`, `idx`, `spell_d0`, `spell_d1`, `spell_d2`, `spell_d3`, `effect_d0`, `effect_d1`, `effect_d2`, `effect_d3`, `first_ms`, `first_max_ms`, `period_ms`, `period_max_ms`, `hp_pct`, `target`, `comment`)
SELECT 11988, 5, 2105825, 2105825, 2105825, 2105825, 0, 0, 0, 0, 13000, 0, 45100, 0, 0, 3, CONVERT(X'4361766520496e2c20342073206166746572204d6173736976652053746f6d70205b636f727075735d' USING utf8mb4) WHERE NOT EXISTS (SELECT 1 FROM `coa_boss_schedule` WHERE `entry` = 11988 AND `idx` = 5);

INSERT INTO `coa_boss_schedule` (`entry`, `idx`, `spell_d0`, `spell_d1`, `spell_d2`, `spell_d3`, `effect_d0`, `effect_d1`, `effect_d2`, `effect_d3`, `first_ms`, `first_max_ms`, `period_ms`, `period_max_ms`, `hp_pct`, `target`, `comment`)
SELECT 12118, 4, 2105201, 2105201, 2105201, 2105201, 2105205, 2105205, 2105205, 2105205, 20900, 27500, 22200, 22600, 0, 3, CONVERT(X'496d70656e64696e6720446f6f6d205b6c6f67733a2066697273742032302e392d32372e3520732c207265706561742032322e322d32322e3620735d' USING utf8mb4) WHERE NOT EXISTS (SELECT 1 FROM `coa_boss_schedule` WHERE `entry` = 12118 AND `idx` = 4);

UPDATE `coa_boss_schedule` SET `effect_d0` = 2105748, `effect_d1` = 2105748, `effect_d2` = 2105748, `effect_d3` = 2105748
WHERE `entry` = 12056 AND `idx` = 7 AND `spell_d0` = 2105747 AND `effect_d0` = 0 AND `effect_d1` = 0 AND `effect_d2` = 0 AND `effect_d3` = 0;

UPDATE `coa_boss_schedule` SET `effect_d0` = 2105219, `effect_d1` = 2105219, `effect_d2` = 2105219, `effect_d3` = 2105219, `comment` = CONVERT(X'5375707072657373696e6720536861646f7773205b64657369676e65643a206c616e64696e67207370656c6c5d' USING utf8mb4)
WHERE `entry` = 12118 AND `idx` = 3 AND `spell_d0` = 2105218 AND `effect_d0` = 0 AND `effect_d1` = 0 AND `effect_d2` = 0 AND `effect_d3` = 0 AND `comment` = CONVERT(X'5375707072657373696e6720536861646f7773' USING utf8mb4);

UPDATE `coa_boss_schedule` SET `target` = 0
WHERE `entry` = 12264 AND `idx` = 2 AND `spell_d0` = 2105607 AND `target` = 2;

UPDATE `coa_boss_schedule` SET `target` = 3
WHERE `entry` = 12264 AND `idx` = 5 AND `spell_d0` = 2105609 AND `target` = 2;

COMMIT;
