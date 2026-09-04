#ifndef MOD_GUILDENCLAVE_DEFINES_H
#define MOD_GUILDENCLAVE_DEFINES_H

#include <cstdint>

#include "Player.h"
#include "Guild.h"

/* Guild Ranks:
    GR_GUILDMASTER  = 0,
    GR_OFFICER      = 1,
    GR_VETERAN      = 2,
    GR_MEMBER       = 3,
    GR_INITIATE     = 4
*/

enum GHMapPosition
{
    GH_MAP_NORTH,
    GH_MAP_EAST,
    GH_MAP_SOUTH,
    GH_MAP_NORTH
}

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
// Purchasing and selling
// =====================================================
enum GuildEnclaveActions
{
    ACTION_NONE           = 0,
    ACTION_TELEPORT       = 1,
    ACTION_SELL           = 2,
    ACTION_PREBUILT_START = 100000,
    ACTION_BUY_START      = 200000,
    ACTION_CATEGORY_START = 300000,
    ACTION_CATALOG_START  = 400000,
    ACTION_BACK           = 500000,
    ACTION_CONFIRM        = 600000
};

#endif
