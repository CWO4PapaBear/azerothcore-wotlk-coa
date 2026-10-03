CREATE TABLE IF NOT EXISTS `area52_build` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `owner_account` int unsigned NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `archived` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `owner_archive` (`owner_account`, `archived`, `id`),
  CONSTRAINT `area52_build_archive_valid` CHECK (`archived` IN (0, 1))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_build_revision` (
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  `format_version` smallint unsigned NOT NULL DEFAULT 1,
  `category` int unsigned NOT NULL,
  `name` varchar(128) NOT NULL,
  `document` json NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`build_id`, `revision`),
  KEY `category_build` (`category`, `build_id`, `revision`),
  CONSTRAINT `area52_revision_build` FOREIGN KEY (`build_id`) REFERENCES `area52_build` (`id`) ON DELETE CASCADE,
  CONSTRAINT `area52_revision_positive` CHECK (`revision` > 0 AND `format_version` > 0),
  CONSTRAINT `area52_document_object` CHECK (JSON_TYPE(`document`) = 'OBJECT'),
  CONSTRAINT `area52_document_size` CHECK (JSON_STORAGE_SIZE(`document`) <= 65536)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_build_featured` (
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  `featured_by_account` int unsigned NOT NULL,
  `featured_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`build_id`),
  CONSTRAINT `area52_featured_revision` FOREIGN KEY (`build_id`, `revision`)
    REFERENCES `area52_build_revision` (`build_id`, `revision`) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
