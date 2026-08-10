#ifndef MOD_GUILDHOUSE_MGR_H
#define MOD_GUILDHOUSE_MGR_H

#include <unordered_map>
#include <vector>
#include <cstdint>

#include "GuildHouseTypes.h"

class Player;

class GuildHouseMgr
{
public:

    static GuildHouseMgr& Instance();

    void Load();

    // =====================================================
    // Phase
    // =====================================================
    uint32_t GetPhaseMask(uint32_t guildId) const;
    bool HasPhase(uint32_t guildId) const;
    uint32_t GeneratePhaseMask(uint32_t locationId);
    uint32_t CreatePhase(uint32_t guildId, uint32_t locationId);
    bool RemovePhase(uint32_t guildId);
    bool EnterPhase(Player* player, uint32_t guildId);
    bool LeavePhase(Player* player);

    // =====================================================
    // Guild House Ownership
    // =====================================================
    bool IsMember(Player* player) const;
    bool AddMember(uint32_t guildId, uint64_t guid);
    bool RemoveMember(uint32_t guildId, uint64_t guid);

    // =====================================================
    // Guild House Ownership
    // =====================================================
    bool HasGuildHouse(uint32_t guildId) const;
    const std::unordered_map<uint32_t, GHGuildHouse>& GetHouses() const;
    const GHGuildHouse* GetGuildHouse(uint32_t guildId) const;
    GHGuildHouse* GetGuildHouse(uint32_t guildId);
    const GHLocation* GetGuildLocation(uint32_t guildId) const;

    // =====================================================
    // Create and Sell Guild House
    // =====================================================
    bool CreateGuildHouse(Player* player, uint32_t guildId, uint32_t ownerGuid, uint32_t locationId);
    bool SellGuildHouse(uint32_t guildId);

    // =====================================================
    // Money Management
    // =====================================================
    bool HasEnoughMoneyInGuild(uint32_t guildId, uint64_t amount);
    bool RemoveMoneyFromGuild(uint32_t guildId, uint64_t amount);
    bool AddMoneyToGuild(uint32_t guildId, uint64_t amount);

    // =====================================================
    // Teleport
    // =====================================================
    bool TeleportToGuildHouse(Player* player);

    // =====================================================
    // Locations
    // =====================================================
    const GHLocation* GetLocation(uint32_t locationId) const;
    GHLocation* GetLocation(uint32_t locationId);
    std::vector<const GHLocation*> GetLocations() const;

    // =====================================================
    // Boundary
    // =====================================================
    bool IsInsideGuildHouseBoundary(uint32_t guildId, float x, float y) const;
    bool CheckBoundary(Player* player);

    // =====================================================
    // Assets
    // =====================================================
    const GHGuildAsset* GetAsset(uint32_t guildId, uint32_t assetId) const;
    GHGuildAsset* GetAsset(uint32_t guildId, uint32_t assetId);
    std::vector<const GHGuildAsset*> GetPurchasedAssets(uint32_t guildId) const;
    bool PlaceAsset(Player* player, uint32_t assetId);
    bool MoveAsset(Player* player, uint32_t assetId);
    bool StoreAsset(Player* player, uint32_t assetId);
    bool SellAsset(Player* player, uint32_t assetId);

    // =====================================================
    // Catalog
    // =====================================================
    bool PurchaseCatalogItem(Player* player, uint32_t catalogId);

    // =====================================================
    // Salesman
    // =====================================================
    bool HasSalesman(uint32_t guildId) const;
    bool CreatePermanentSalesman(Player* player, uint32_t entry);
    //void RecordSalesmanSpawn(uint32_t guildId, uint32_t spawnId, uint32_t mapId, uint32_t phaseMask, float x, float y, float z, float o);

private:

    GuildHouseMgr() = default;

    uint32_t _nextAssetId = 1;

    std::unordered_map<uint32_t, GHGuildHouse> _houses;
    std::unordered_map<uint32_t, GHLocation> _locations;

};

#define sGuildHouseMgr GuildHouseMgr::Instance()

#endif
