DROP TABLE IF EXISTS `guildenclave`;
CREATE TABLE `guildenclave`
(
    `guildId` INT UNSIGNED NOT NULL,
    `ownerGuid` INT UNSIGNED NOT NULL,
    `faction` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `requiredGuildRank` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `locationId` INT UNSIGNED NOT NULL DEFAULT 0,
    `purchasePrice` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `purchaseDate` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`guildId`),
    KEY `idx_owner` (`ownerGuid`)
) ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_unicode_ci;

DROP TABLE IF EXISTS `guildenclave_asset`;
CREATE TABLE `guildenclave_asset`
(
    `assetId` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `guildId` INT UNSIGNED NOT NULL,
    `layoutId` SMALLINT UNSIGNED NOT NULL DEFAULT 1,
    `catalogId` INT UNSIGNED NOT NULL,
    `purchasePrice` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `status` TINYINT UNSIGNED NOT NULL DEFAULT 0,

    `positionX` FLOAT NOT NULL,
    `positionY` FLOAT NOT NULL,
    `positionZ` FLOAT NOT NULL,
    `orientation` FLOAT NOT NULL,

    `wander` INT NOT NULL,
    
    `createdBy` INT UNSIGNED NOT NULL,
    `enabled` TINYINT(1) NOT NULL DEFAULT 1,
    `createdDate` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`assetId`),
    KEY `idx_guild` (`guildId`),
    KEY `idx_layout` (`layoutId`),
    KEY `idx_catalog` (`catalogId`),
    KEY `idx_status` (`status`)
) ENGINE=InnoDB
DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_unicode_ci;

DROP TABLE IF EXISTS guildenclave_spawn;
CREATE TABLE guildenclave_spawn
(
    spawnId INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    guildId INT UNSIGNED NOT NULL,
    assetId INT UNSIGNED NOT NULL,
    phaseMask INT UNSIGNED NOT NULL,
    spawnGuid INT UNSIGNED NOT NULL,
    spawnType TINYINT NOT NULL,
    mapId INT UNSIGNED NOT NULL,

    x FLOAT,
    y FLOAT,
    z FLOAT,
    o FLOAT,
    w INT
);

