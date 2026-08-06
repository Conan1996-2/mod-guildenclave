#ifndef MOD_GUILDHOUSE_CONFIG_H
#define MOD_GUILDHOUSE_CONFIG_H

#include <cstdint>

class GuildHouseConfig
{
public:

    static GuildHouseConfig& Instance();

    void Load();

    bool IsEnabled() const;
    bool IsFree() const;
    bool LoadAll() const;
    bool AllWander() const;

    int16_t WanderDistance() const;

    double GetRefundPercent() const;

private:

    GuildHouseConfig() = default;

    bool _enabled = false;
    bool _allWander = false;

    uint8_t _free = false;

    int16_t _wanderDistance = 0;

    double _refundPercent = 0;
};

#define sGuildHouseConfig GuildHouseConfig::Instance()

#endif
