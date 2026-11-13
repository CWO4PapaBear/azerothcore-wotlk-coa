-- Strict mode rejects narrowing if any existing name would be truncated.
SET SESSION sql_mode=CONCAT_WS(',',NULLIF(@@SESSION.sql_mode,''),'STRICT_ALL_TABLES');
ALTER TABLE character_pet MODIFY COLUMN name VARCHAR(21) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'Pet';
