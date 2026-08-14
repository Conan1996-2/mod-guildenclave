#ifndef MOD_GUILDENCLAVE_UTIL_H
#define MOD_GUILDENCLAVE_UTIL_H

#include <cstdint>
#include <string>

class Player;

namespace GuildEnclaveUtil
{
    // =====================================================
    // Bit flag helper
    // =====================================================
    bool HasFlag(uint32_t value, uint32_t flag);

    // =====================================================
    // Faction flag helper
    // =====================================================
    bool IsAlliance(uint32_t flags);
    bool IsHorde(uint32_t flags);
    bool IsNeutral(uint32_t flags);

    // =====================================================
    // Guild Enclave checks
    // =====================================================
    bool IsInsideGuildEnclaveBoundary(Player* player);
    bool IsInGuildEnclaveArea(Player* player);
    bool IsInGuildEnclavePhase(Player* player);
    bool IsInGuildEnclave(Player* player);
    bool HasGuildEnclave(Player* player);

    // =====================================================
    // Guild permissions
    // =====================================================
    bool IsGuildRank(Player* player);
    inline bool CanManageGuildEnclave(Player* player)
    {
        return IsGuildRank(player) && IsInGuildEnclave(player);
    }

    // =====================================================
    // Money Calculations
    // =====================================================
    std::string GoldToString (uint64_t price);
}

#endif
