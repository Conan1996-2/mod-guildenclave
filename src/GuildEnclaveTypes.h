#ifndef MOD_GUILDENCLAVE_TYPES_H
#define MOD_GUILDENCLAVE_TYPES_H

#include <cstdint>
#include <string>
#include <vector>
#include <type_traits>
#include <unordered_set>

#include "GuildEnclaveDefines.h"

// =====================================================
// Catalog Component
// =====================================================
struct GHCatalogAsset
{
    uint32_t ComponentId = 0;
    uint32_t CatalogId = 0;
    uint32_t Entry = 0;
    uint32_t DisplayId = 0;
    float Scale = 1.0f;

    GHSpawnFlags SpawnFlags = GH_SPAWN_NONE;
    GHBehaviorFlags BehaviorFlags = GH_BEHAVIOR_NONE;

    float XOffset = 0.0f;
    float YOffset = 0.0f;
    float ZOffset = 0.0f;
    float OOffset = 0.0f;

    uint16_t w = 0;

    uint32_t ChildCatalogId = 0;
    uint16_t SortOrder = 0;
};

// =====================================================
// Catalog Category
// =====================================================
struct GHCategory
{
    uint32_t Id = 0;
    uint32_t ParentId = 0;
    std::string Name;
    uint16_t SortOrder = 0;
    bool Enabled = false;
};

// =====================================================
// Catalog Item
// =====================================================
struct GHCatalog
{
    uint32_t CatalogId = 0;
    uint32_t CategoryId = 0;
    std::string Name;

    uint32_t Price = 0;

    GHSpawnFlags SpawnFlags = GH_SPAWN_NONE;
    GHBehaviorFlags BehaviorFlags = GH_BEHAVIOR_NONE;

    bool Enabled = false;

    std::vector<GHCatalogAsset> Components;
};

// =====================================================
// Purchased Guild Asset
//
// Stored in guildenclave_asset
//
// Phase based.
// =====================================================
struct GHGuildAsset
{
    uint32_t AssetId = 0;
    uint32_t GuildId = 0;
    uint32_t CatalogId = 0;
    uint16_t LayoutId = 0;

    uint64_t PurchasePrice = 0;
    GHAssetStatus Status = GH_ASSET_PURCHASED;

    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float O = 0.0f;

    uint16_t w = 0;
};

// =====================================================
// Existing Spawns
//
// Stored in guildenclave_spawn
//
// Phase based.
// =====================================================
struct GHGuildSpawn
{
    uint32_t SpawnId = 0;
    uint32_t GuildId = 0;
    uint32_t AssetId = 0;
    uint32_t PhaseMask = 0;
    uint32_t SpawnGuid = 0;
    uint8_t SpawnType = 0;

    uint32_t MapId = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float O = 0.0f;

    int16_t w = 0;
};

// =====================================================
// Permanent Spawn Registry
//
// Stored in guildenclave_spawn
//
// Phase based.
// =====================================================
struct GHSpawnRecord
{
    uint32_t SpawnId = 0;
    uint32_t GuildId = 0;
    uint32_t AssetId = 0;
    uint32_t CatalogId = 0;
    uint32_t Guid = 0;

    GHSpawnFlags Type = GH_SPAWN_NONE;

    uint32_t MapId = 0;
    uint32_t PhaseMask = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float O = 0.0f;

    uint16_t w = 0;
};

// =====================================================
// Locations and constraints for each Guild Enclave
// =====================================================
struct GHLocation
{
    uint32_t Id = 0;
    std::string Name;

    uint32_t MapId = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float O = 0.0f;

    float MinX = 0.0f;
    float MaxX = 0.0f;
    float MinY = 0.0f;
    float MaxY = 0.0f;

    uint64_t Price = 0;

    uint32_t InUseBitMask = 0;

    bool AllowPhasing = false;
    bool Enabled = false;
};

// =====================================================
// Guild Enclave Ownership
//
// One Enclave per guild.
//
// Phase owns the Enclave.
// =====================================================
struct GHGuildEnclave
{
    uint32_t GuildId = 0;
    uint8_t Faction = 0;

    uint32_t OwnerGuid = 0;
    uint8_t RequiredGuildRank = 0;

    uint32_t LocationId = 0;
    uint64_t PurchasePrice = 0;

    uint32_t PhaseMask = 0;

    std::unordered_map<uint32_t, GHGuildAsset> Assets;
    std::unordered_map<uint32_t, uint32_t> AssetIdMap;
    std::vector<GHGuildSpawn> Spawns;

    std::unordered_set<uint64_t> Members;
};

// =====================================================
// Utility Helpers
// =====================================================

namespace GuildEnclaveUtil
{
    template<typename T> constexpr bool HasFlag(T value, T flag)
    {
        using U = std::underlying_type_t<T>;

        return (static_cast<U>(value) & static_cast<U>(flag)) != 0;
    }
}

#endif
