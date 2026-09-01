#ifndef MOD_GUILDENCLAVE_UTIL_H
#define MOD_GUILDENCLAVE_UTIL_H

#include "GuildEnclaveDefines.h"

#include <cstdint>
#include <string>

class Player;

namespace GuildEnclaveUtil
{
    // =====================================================
    // Bit flag helper
    // =====================================================
    inline bool HasFlag(uint32_t value, uint32_t flag) { return (value & flag) != 0; }

    // =====================================================
    // Faction flag helper
    // =====================================================
    inline bool IsAlliance(uint32_t flags) { return HasFlag(flags, GH_FACTION_ALLIANCE); }
    inline bool IsHorde(uint32_t flags) { return HasFlag(flags, GH_FACTION_HORDE); }
    inline bool IsNeutral(uint32_t flags) { return HasFlag(flags, GH_FACTION_NEUTRAL); }

    // =====================================================
    // Guild Enclave checks
    // =====================================================
    uint32_t GetPlayerLocationId(Player* player);
    bool IsInsideGuildEnclaveBoundary(Player* player);
    bool IsInGuildEnclaveArea(Player* player);
    bool IsInGuildEnclavePhase(Player* player);
    bool IsInGuildEnclave(Player* player);
    bool HasGuildEnclave(Player* player);

    // =====================================================
    // Guild permissions
    // =====================================================
    bool IsGuildRank(Player* player);
    inline bool CanManageGuildEnclave(Player* player) { return IsGuildRank(player) && IsInGuildEnclave(player); }

    // =====================================================
    // Money Calculations
    // =====================================================
    std::string GoldToString (uint64_t price);
}

#endif
