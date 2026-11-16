DELETE FROM `rbac_linked_permissions` WHERE `id` = 195 AND `linkedId` IN (28, 29);
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (195, 28), (195, 29);
