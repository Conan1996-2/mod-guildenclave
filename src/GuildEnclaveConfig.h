#ifndef MOD_GUILDENCLAVE_CONFIG_H
#define MOD_GUILDENCLAVE_CONFIG_H

#include <cstdint>

class GuildEnclaveConfig
{
public:

    static GuildEnclaveConfig& Instance();

    void Load();

    bool IsEnabled() const;
    bool IsFree() const;
    bool LoadAll() const;
    bool AllWander() const;

    int16_t WanderDistance() const;

    double GetRefundPercent() const;

    const std::unordered_map<uint32_t, uint32_t> GetFreeObjects() { return _freeObjects; } const;

private:

    GuildEnclaveConfig() = default;

    bool _enabled = false;
    bool _allWander = false;

    uint8_t _free = false;

    int16_t _wanderDistance = 0;

    double _refundPercent = 0;

    std::vector<uint32_t> _startingObjects;
};

#define sGuildEnclaveConfig GuildEnclaveConfig::Instance()

#endif
