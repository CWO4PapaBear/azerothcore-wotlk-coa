SET FOREIGN_KEY_CHECKS=0;

CREATE TABLE `area52_account_friend` (
  `account` int unsigned NOT NULL,
  `friend_account` int unsigned NOT NULL,
  `original_guid` int unsigned NOT NULL,
  `original_name` varchar(48) COLLATE utf8mb4_unicode_ci NOT NULL,
  `note` varchar(48) COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`account`,`friend_account`),
  KEY `friend_account` (`friend_account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_account_friend_migration` (
  `id` tinyint unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_account_gift_campaign` (
  `id` int unsigned NOT NULL,
  `enabled` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_account_gift_delivery` (
  `campaign` int unsigned NOT NULL,
  `account` int unsigned NOT NULL,
  `receiver` int unsigned NOT NULL,
  `item_guid` int unsigned NOT NULL,
  `mail_id` int unsigned NOT NULL,
  `delivered_at` int unsigned NOT NULL,
  PRIMARY KEY (`campaign`,`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_archetype_vanity_delivery` (
  `guid` int unsigned NOT NULL,
  `delivered_at` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE `area52_build` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `owner_account` int unsigned NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `archived` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `owner_archive` (`owner_account`,`archived`,`id`),
  CONSTRAINT `area52_build_archive_valid` CHECK ((`archived` in (0,1)))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_build_archetype` (
  `archetype_id` int unsigned NOT NULL,
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  PRIMARY KEY (`archetype_id`),
  UNIQUE KEY `area52_archetype_build` (`build_id`),
  KEY `area52_archetype_revision` (`build_id`,`revision`),
  CONSTRAINT `area52_archetype_revision` FOREIGN KEY (`build_id`, `revision`) REFERENCES `area52_build_revision` (`build_id`, `revision`) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_build_featured` (
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  `featured_by_account` int unsigned NOT NULL,
  `featured_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`build_id`),
  KEY `area52_featured_revision` (`build_id`,`revision`),
  CONSTRAINT `area52_featured_revision` FOREIGN KEY (`build_id`, `revision`) REFERENCES `area52_build_revision` (`build_id`, `revision`) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_build_revision` (
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  `format_version` smallint unsigned NOT NULL DEFAULT '1',
  `category` int unsigned NOT NULL,
  `name` varchar(128) COLLATE utf8mb4_unicode_ci NOT NULL,
  `document` json NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`build_id`,`revision`),
  KEY `category_build` (`category`,`build_id`,`revision`),
  CONSTRAINT `area52_revision_build` FOREIGN KEY (`build_id`) REFERENCES `area52_build` (`id`) ON DELETE CASCADE,
  CONSTRAINT `area52_document_object` CHECK ((json_type(`document`) = _latin1'OBJECT')),
  CONSTRAINT `area52_document_size` CHECK ((json_storage_size(`document`) <= 65536)),
  CONSTRAINT `area52_revision_positive` CHECK (((`revision` > 0) and (`format_version` > 0)))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_build_trophy_claim` (
  `account` int unsigned NOT NULL,
  `build_id` bigint unsigned NOT NULL,
  `completion_guid` int unsigned NOT NULL,
  `completion_revision` int unsigned NOT NULL,
  `mail_id` int unsigned NOT NULL,
  `claimed_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`account`,`build_id`),
  UNIQUE KEY `mail_id` (`mail_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_draft_pet` (
  `guid` int unsigned NOT NULL,
  `pet_number` int unsigned DEFAULT NULL,
  `previous_pet` int unsigned NOT NULL DEFAULT '0',
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  `permanent` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  UNIQUE KEY `draft_pet_number` (`pet_number`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_draft_reward` (
  `guid` int unsigned NOT NULL,
  `account` int unsigned NOT NULL,
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_featured_milestones` (
  `guid` int unsigned NOT NULL,
  `level` tinyint unsigned NOT NULL,
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  `mail_id` int unsigned NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`guid`,`level`),
  UNIQUE KEY `mail_id` (`mail_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_library_import` (
  `public_id` char(36) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `build_id` bigint unsigned NOT NULL,
  `public_revision` int unsigned NOT NULL,
  `native_revision` int unsigned NOT NULL,
  `record_sha256` char(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `wire_sha256` char(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `created_epoch` bigint unsigned NOT NULL,
  PRIMARY KEY (`public_id`),
  UNIQUE KEY `area52_library_build` (`build_id`),
  KEY `area52_library_revision` (`build_id`,`native_revision`),
  CONSTRAINT `area52_library_revision` FOREIGN KEY (`build_id`, `native_revision`) REFERENCES `area52_build_revision` (`build_id`, `revision`) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_library_publication` (
  `owner_account` int unsigned NOT NULL,
  `request_token` varchar(24) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  `wire` mediumtext CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `public_alias` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `consent` varchar(16) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `state` enum('queued','accepted','published','failed','conflict') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'queued',
  `queue_id` char(36) CHARACTER SET ascii COLLATE ascii_bin DEFAULT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`owner_account`,`request_token`),
  KEY `publication_pending` (`state`,`created_at`),
  CONSTRAINT `library_publication_consent` CHECK ((`consent` = _latin1'CC0-1.0')),
  CONSTRAINT `library_publication_wire_limit` CHECK ((length(`wire`) <= 49152))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_library_refresh` (
  `id` tinyint unsigned NOT NULL,
  `generation` bigint unsigned NOT NULL DEFAULT '0',
  `state` varchar(16) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT 'idle',
  `requested_at` bigint unsigned NOT NULL DEFAULT '0',
  `finished_at` bigint unsigned NOT NULL DEFAULT '0',
  `added` int unsigned NOT NULL DEFAULT '0',
  `updated` int unsigned NOT NULL DEFAULT '0',
  `skipped` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  CONSTRAINT `area52_library_refresh_state` CHECK ((`state` in (_latin1'idle',_latin1'queued',_latin1'running',_latin1'complete',_latin1'failed'))),
  CONSTRAINT `area52_library_single_refresh` CHECK ((`id` = 1))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_test_boost_campaign` (
  `id` int unsigned NOT NULL,
  `enabled` tinyint unsigned NOT NULL DEFAULT '0',
  `use_enabled` tinyint unsigned NOT NULL DEFAULT '0',
  `new_character_guid_floor` int unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_test_boost_legacy_account` (
  `account` int unsigned NOT NULL,
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `area52_tester_cache_delivery` (
  `guid` int unsigned NOT NULL,
  `delivered_at` int unsigned NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE `character_callboard_contract` (
  `guid` int unsigned NOT NULL,
  `contract_type` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `week_start` int unsigned NOT NULL,
  `completions` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`contract_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `coa_wildcard_skill_card` (
  `account` int unsigned NOT NULL,
  `card` int unsigned NOT NULL,
  `progress` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`card`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `coa_wildcard_skill_card_account` (
  `account` int unsigned NOT NULL,
  `bonus_progress` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `coa_wildcard_skill_card_pending` (
  `account` int unsigned NOT NULL,
  `id` int unsigned NOT NULL,
  `card` int unsigned NOT NULL,
  PRIMARY KEY (`account`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `coa_wildcard_skill_card_purchase` (
  `account` int unsigned NOT NULL,
  `type` tinyint unsigned NOT NULL,
  `count` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `coa_wildcard_specialization_cache` (
  `account` int unsigned NOT NULL,
  `claimed_at` int unsigned NOT NULL,
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `mod_area52_cache_attempt` (
  `cache_guid` bigint unsigned NOT NULL,
  `owner_guid` bigint unsigned NOT NULL,
  `request_id` bigint unsigned NOT NULL,
  `revision` bigint unsigned NOT NULL,
  `phase` tinyint unsigned NOT NULL,
  `payload` varbinary(41000) NOT NULL,
  PRIMARY KEY (`cache_guid`),
  UNIQUE KEY `uq_cache_request` (`request_id`),
  KEY `idx_cache_owner` (`owner_guid`),
  CONSTRAINT `chk_cache_phase` CHECK ((`phase` <= 3))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `mod_area52_cache_attempt_guard` (
  `request_id` bigint unsigned NOT NULL,
  `matched_rows` int NOT NULL,
  PRIMARY KEY (`request_id`),
  CONSTRAINT `chk_cache_attempt_match` CHECK ((`matched_rows` = 1))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

SET FOREIGN_KEY_CHECKS=1;

ALTER TABLE character_aura ADD sourceAuraSpell int unsigned NOT NULL DEFAULT 0 AFTER remainCharges;

ALTER TABLE character_pet MODIFY name varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'Pet';
