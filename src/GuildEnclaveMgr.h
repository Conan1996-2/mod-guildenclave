#ifndef MOD_GUILDENCLAVE_MGR_H
#define MOD_GUILDENCLAVE_MGR_H

#include <unordered_map>
#include <vector>
#include <cstdint>

#include "GuildEnclaveTypes.h"

class Player;

class GuildEnclaveMgr
{
public:

    static GuildEnclaveMgr& Instance();

    void Load();

    // =====================================================
    // Phase
    // =====================================================
    uint32_t GetPhaseMask(uint32_t guildId) const;
    bool HasPhase(uint32_t guildId) const;
    uint32_t GeneratePhaseMask(uint32_t locationId);
    uint32_t CreatePhase(uint32_t guildId, uint32_t locationId);
    bool RemovePhase(uint32_t guildId);
    bool EnterPhase(Player* player);
    bool LeavePhase(Player* player);

    // =====================================================
    // Guild Enclave Membership
    // =====================================================
    bool IsMember(Player* player) const;
    bool AddMember(Player* player);
    bool RemoveMember(Player* player);

    // =====================================================
    // Guild Enclave Ownership
    // =====================================================
    bool HasGuildEnclave(uint32_t guildId) const;
    const std::unordered_map<uint32_t, GHGuildEnclave>& GetHouses() const;
    const GHGuildEnclave* GetGuildEnclave(uint32_t guildId) const;
    GHGuildEnclave* GetGuildEnclave(uint32_t guildId);

    // =====================================================
    // Create and Sell Guild Enclave
    // =====================================================
    bool CreateGuildEnclave(Player* player, uint32_t guildId, uint32_t ownerGuid, uint32_t locationId);
    bool SellGuildEnclave(uint32_t guildId);

    // =====================================================
    // Money Management
    // =====================================================
    bool HasEnoughMoneyInGuild(uint32_t guildId, uint64_t amount);
    bool RemoveMoneyFromGuild(uint32_t guildId, uint64_t amount);
    bool AddMoneyToGuild(uint32_t guildId, uint64_t amount);

    // =====================================================
    // Teleport
    // =====================================================
    bool TeleportToGuildEnclave(Player* player);

    // =====================================================
    // Locations
    // =====================================================
    const GHLocation* GetLocation(uint32_t locationId) const;
    GHLocation* GetLocation(uint32_t locationId);
    std::vector<const GHLocation*> GetLocations() const;

    // =====================================================
    // Boundary
    // =====================================================
    bool IsInsideGuildEnclaveBoundary(uint32_t guildId, float x, float y) const;
    bool CheckBoundary(Player* player);
    const GHLocation* GetGuildLocation(uint32_t guildId) const;

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
    bool WanderAsset(Player* player, uint32_t assetId);

    // =====================================================
    // Catalog
    // =====================================================
    bool PurchaseCatalogItem(Player* player, uint32_t catalogId);

private:

    GuildEnclaveMgr() = default;

    uint32_t _nextAssetId = 1;

    std::unordered_map<uint32_t, GHGuildEnclave> _houses;
    std::unordered_map<uint32_t, GHLocation> _locations;

};

#define sGuildEnclaveMgr GuildEnclaveMgr::Instance()

#endif
