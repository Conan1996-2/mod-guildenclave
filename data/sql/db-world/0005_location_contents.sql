TRUNCATE TABLE `guildenclave_locations`;

INSERT INTO `guildenclave_locations` (`name`,`mapId`,`zoneId`,`areaId`,`positionX`,`positionY`,`positionZ`,`orientation`,`minX`,`maxX`,`minY`,`maxY`,`price`,`enabled`) VALUES
('GM Island', 1, 876, 876, 16222.57, 16265.91, 13.21, 0, 16144, 16393, 16139, 16538, 1000000, 1),
('Valormok', 37, 268, 268, 88.38, 915.82, 338.12, 0, -200, 460, 460, 1250, 10000000, 1),
('Talrendis Outpost', 37, 268, 268, 1144.43, 240, 353, 0, 665, 1347, -165, 534, 10000000, 1);
