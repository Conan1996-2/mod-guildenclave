DROP TABLE IF EXISTS `guildenclave_category`;
CREATE TABLE `guildenclave_category`
(
    `categoryId` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `parentId` INT UNSIGNED NOT NULL DEFAULT 0,
    `name` VARCHAR(80) NOT NULL,
    `sortOrder` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    `enabled` TINYINT(1) NOT NULL DEFAULT 1,

    PRIMARY KEY (`categoryId`),
    KEY `idx_parent` (`parentId`)
) ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_unicode_ci;

DROP TABLE IF EXISTS `guildenclave_catalog`;
CREATE TABLE `guildenclave_catalog`
(
    `catalogId` INT UNSIGNED NOT NULL,
    `categoryId` INT UNSIGNED NOT NULL,

    `name` VARCHAR(100) NOT NULL,

    `price` BIGINT UNSIGNED NOT NULL DEFAULT 0,

    `spawnFlags` INT UNSIGNED NOT NULL DEFAULT 0,
    `behaviorFlags` INT UNSIGNED NOT NULL DEFAULT 0,

    `enabled` TINYINT(1) NOT NULL DEFAULT 1,

    KEY `idx_category` (`categoryId`),
    KEY `idx_enabled` (`enabled`)
) ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_unicode_ci;

DROP TABLE IF EXISTS `guildenclave_prebuilt`;
CREATE TABLE `guildenclave_prebuilt`
(
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `locationId` INT UNSIGNED NOT NULL,
    `catalogId` INT UNSIGNED NOT NULL,
    `X` FLOAT NOT NULL DEFAULT 0,
    `Y` FLOAT NOT NULL DEFAULT 0,
    `Z` FLOAT NOT NULL DEFAULT 0,
    `O` FLOAT NOT NULL DEFAULT 0,
    `W` INT NOT NULL DEFAULT 0,

    PRIMARY KEY (`id`),
    
    KEY `idx_location` (`locationId`)
) ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_unicode_ci;

DROP TABLE IF EXISTS `guildenclave_catalog_asset`;
CREATE TABLE `guildenclave_catalog_asset`
(
    `componentId` INT UNSIGNED NOT NULL AUTO_INCREMENT,

    `catalogId` INT UNSIGNED NOT NULL,

    `spawnFlags` INT UNSIGNED NOT NULL DEFAULT 0,
    `behaviorFlags` INT UNSIGNED NOT NULL DEFAULT 0,

    `entryId` INT UNSIGNED NOT NULL DEFAULT 0,
    `displayId` INT UNSIGNED NOT NULL DEFAULT 0,

    `scale` FLOAT NOT NULL DEFAULT 1.0,

    `xOffset` FLOAT NOT NULL DEFAULT 0,
    `yOffset` FLOAT NOT NULL DEFAULT 0,
    `zOffset` FLOAT NOT NULL DEFAULT 0,
    `oOffset` FLOAT NOT NULL DEFAULT 0,

    `wander` INT NOT NULL DEFAULT 0,

    `childCatalogId` INT UNSIGNED DEFAULT NULL,

    `sortOrder` SMALLINT UNSIGNED NOT NULL DEFAULT 0,

    PRIMARY KEY (`componentId`),

    KEY `idx_catalog` (`catalogId`),
    KEY `idx_child` (`childCatalogId`)
) ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_unicode_ci;
