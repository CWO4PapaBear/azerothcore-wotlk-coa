INSERT IGNORE INTO `spell_group_stack_rules` (`group_id`, `stack_rule`)
SELECT 1078, 3
WHERE NOT EXISTS (SELECT 1 FROM `spell_group_stack_rules` WHERE `group_id` = 1078)
    AND EXISTS (SELECT 1 FROM `spell_group` WHERE `id` = 1078 AND `spell_id` = 1126)
    AND EXISTS (SELECT 1 FROM `spell_group` WHERE `id` = 1078 AND `spell_id` = 523478)
    AND EXISTS (SELECT 1 FROM `spell_group` WHERE `id` = 1078 AND `spell_id` = 523495)
    AND NOT EXISTS (SELECT 1 FROM `spell_group`
        WHERE `id` = 1078 AND `spell_id` NOT IN (1126, 523478, 523495));
