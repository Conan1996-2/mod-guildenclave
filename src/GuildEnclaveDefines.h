#ifndef MOD_GUILDENCLAVE_DEFINES_H
#define MOD_GUILDENCLAVE_DEFINES_H

#include <cstdint>

#include "Player.h"
#include "Guild.h"

// =====================================================
// Spawn Flags
// =====================================================
enum GHSpawnFlags : uint32_t
{
    GH_SPAWN_NONE        = 0,
    GH_SPAWN_CREATURE    = 1 << 0,
    GH_SPAWN_GAMEOBJECT  = 1 << 1,
};

// =====================================================
// Behavior Flags
// =====================================================
enum GHBehaviorFlags : uint32_t
{
    GH_BEHAVIOR_NONE = 0,
    GH_FACTION_ALLIANCE = 1 << 0,
    GH_FACTION_HORDE    = 1 << 1,
    GH_FACTION_NEUTRAL  = 1 << 2,
};

// =====================================================
// Asset Status
// =====================================================
enum GHAssetStatus : uint8_t
{
    GH_ASSET_PURCHASED = 0,
    GH_ASSET_PLACED    = 1,
    GH_ASSET_STORED    = 2,
    GH_ASSET_DISABLED  = 3
};

// =====================================================
// Script Types
// =====================================================
enum GHScriptType : uint32_t
{
    GH_SCRIPT_NONE = 0,

    GH_SCRIPT_PORTAL,
    GH_SCRIPT_BUFF,
    GH_SCRIPT_TRIGGER,
    GH_SCRIPT_MUSIC,
    GH_SCRIPT_CUSTOM
};

// =====================================================
// Utility Helpers
// =====================================================
namespace GuildEnclaveUtil
{
    bool IsGuildEnclavePhase(uint32_t guildId, uint32_t phaseMask);

    //bool IsInGuildEnclave(Player* player);

    bool IsGuildRank(Player* player);

    inline bool HasFlag(uint32_t value, uint32_t flag)
    {
        return (value & flag) != 0;
    }

    inline bool IsAlliance(uint32_t flags)
    {
        return HasFlag(flags, GH_FACTION_ALLIANCE);
    }

    inline bool IsHorde(uint32_t flags)
    {
        return HasFlag(flags, GH_FACTION_HORDE);
    }

    inline bool IsNeutral(uint32_t flags)
    {
        return HasFlag(flags, GH_FACTION_NEUTRAL);
    }

/*
    inline bool IsGuildMaster(Player* player)
    {
        if (!player)
            return false;

        Guild* guild = player->GetGuild();
        if (!guild)
            return false;

        return guild->GetLeaderGUID() == player->GetGUID();
    }
*/

    inline bool CanManageGuildEnclave(Player* player)
    {
        return IsGuildRank(player) && IsInGuildEnclave(player);
    }
}

#endif
