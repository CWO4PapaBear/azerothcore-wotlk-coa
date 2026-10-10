DELETE FROM `achievement_category_dbc` WHERE `ID` IN (14921, 14961);
INSERT INTO `achievement_category_dbc` (`ID`, `Parent`, `Name_Lang_enUS`, `Name_Lang_Mask`, `Ui_Order`) VALUES
(14921, 168, 'Lich King Heroic', 16712190, 4),
(14961, 168, 'Secrets of Ulduar 10-Player Raid', 16712190, 7);
