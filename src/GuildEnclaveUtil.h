#ifndef MOD_GUILDENCLAVE_UTIL_H
#define MOD_GUILDENCLAVE_UTIL_H

#include <cstdint>

class Player;

namespace GuildEnclaveUtil
{
    // =====================================================
    // Bit flag helper
    // =====================================================
    bool HasFlag(uint32_t value, uint32_t flag);

    // =====================================================
    // Guild permissions
    // =====================================================
    bool IsGuildMaster(Player* player);
    bool CanManageGuildEnclave(Player* player);

    // =====================================================
    // Guild House checks
    // =====================================================
    bool IsInGuildEnclave(Player* player);

    // =====================================================
    // Phase validation
    // =====================================================
    bool IsGuildEnclavePhase(uint32_t guildId, uint32_t phaseMask);
}

#endif
