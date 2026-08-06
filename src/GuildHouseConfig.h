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

    double GetRefundPercent() const;

private:

    GuildHouseConfig() = default;

    bool _enabled = false;
    bool _allWander = true;

    uint8_t _free = false;

    double _refundPercent = 0;
};

#define sGuildHouseConfig GuildHouseConfig::Instance()

#endif
