DROP TABLE IF EXISTS `guildenclave_locations`;
CREATE TABLE `guildenclave_locations`
(
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `name` VARCHAR(100) NOT NULL,

    `mapId` SMALLINT UNSIGNED NOT NULL,
    `zoneId` SMALLINT UNSIGNED NOT NULL,
    `areaId` SMALLINT UNSIGNED NOT NULL,
    `positionX` FLOAT NOT NULL DEFAULT 0,
    `positionY` FLOAT NOT NULL DEFAULT 0,
    `positionZ` FLOAT NOT NULL DEFAULT 0,
    `orientation` FLOAT NOT NULL DEFAULT 0,

    `minX` FLOAT NOT NULL DEFAULT 0,
    `maxX` FLOAT NOT NULL DEFAULT 0,
    `minY` FLOAT NOT NULL DEFAULT 0,
    `maxY` FLOAT NOT NULL DEFAULT 0,

    `price` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `enabled` TINYINT(1) NOT NULL DEFAULT 1,

    PRIMARY KEY(`id`),
    KEY `idx_map` (`mapId`),
    KEY `idx_enabled` (`enabled`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
