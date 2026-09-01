#include "GuildEnclaveMgr.h"

#include "GuildEnclaveDefines.h"
#include "GuildEnclaveConfig.h"
#include "GuildEnclaveUtil.h"
#include "GuildEnclaveSpawner.h"
#include "GuildEnclaveCatalogMgr.h"

#include "ScriptMgr.h"

#include "DatabaseEnv.h"
#include "QueryResult.h"
#include "Player.h"
#include "Chat.h"
#include "Log.h"
#include "GuildMgr.h"

#include <algorithm>
#include <cstdint>

GuildEnclaveMgr& GuildEnclaveMgr::Instance()
{
    static GuildEnclaveMgr instance;
    return instance;
}

// =====================================================
// Load
// =====================================================
void GuildEnclaveMgr::Load()
{
    _houses.clear();
    _locations.clear();

    // -------------------------------------------------
    // Get asset maximum database number
    // -------------------------------------------------
    if(QueryResult result = CharacterDatabase.Query("SELECT MAX(assetId) FROM guildenclave_asset"))
    {
        Field* fields = result->Fetch();
    
        uint32_t maxAssetId = fields[0].Get<uint32>();
        _nextAssetId = maxAssetId + 1;
    }
    
    if(QueryResult result = WorldDatabase.Query("SELECT id,name,mapId,positionX,positionY,positionZ,orientation,minX,maxX,minY,maxY,price,enabled FROM guildenclave_locations"))
    {
        do
        {
            Field* fields = result->Fetch();

            GHLocation location;
            location.Id = fields[0].Get<uint32>();
            location.Name = fields[1].Get<std::string>();
            location.MapId = fields[2].Get<uint32>();
            location.X = fields[3].Get<float>();
            location.Y = fields[4].Get<float>();
            location.Z = fields[5].Get<float>();
            location.O = fields[6].Get<float>();
            location.MinX = fields[7].Get<float>();
            location.MaxX = fields[8].Get<float>();
            location.MinY = fields[9].Get<float>();
            location.MaxY = fields[10].Get<float>();
            location.Price = fields[11].Get<uint64_t>();
            location.Enabled = fields[12].Get<bool>();

            _locations.emplace(location.Id, location);
        }while(result->NextRow());
    }

    if(QueryResult result = CharacterDatabase.Query("SELECT guildId,ownerGuid,faction,requiredGuildRank,locationId,purchasePrice FROM guildenclave"))
    {
        do
        {
            Field* fields = result->Fetch();

            GHGuildEnclave house;
            house.GuildId = fields[0].Get<uint32>();
            house.OwnerGuid = fields[1].Get<uint32>();
            house.Faction =  fields[2].Get<uint8_t>();
            house.RequiredGuildRank = fields[3].Get<uint8_t>();
            house.LocationId = fields[4].Get<uint32>();
            house.PurchasePrice = fields[5].Get<uint64_t>();
            house.PhaseMask = 0;

            _houses.emplace(house.GuildId, house);

        }while(result->NextRow());
    }

    if(QueryResult result = CharacterDatabase.Query("SELECT assetId,guildId,catalogId,purchasePrice,status,positionX,positionY,positionZ,orientation,wander FROM guildenclave_asset"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 guildId = fields[1].Get<uint32>();

            auto itr = _houses.find(guildId);
            if(itr == _houses.end())
                continue;

            GHGuildAsset asset;
            asset.AssetId = fields[0].Get<uint32>();
            asset.GuildId = fields[1].Get<uint32>();
            asset.CatalogId = fields[2].Get<uint32>();
            asset.PurchasePrice = fields[3].Get<uint64_t>();
            asset.Status = static_cast<GHAssetStatus>(fields[4].Get<uint8>());
            asset.X = fields[5].Get<float>();
            asset.Y = fields[6].Get<float>();
            asset.Z = fields[7].Get<float>();
            asset.O = fields[8].Get<float>();
            asset.w = fields[9].Get<int16_t>();

            //itr->second.Assets.push_back(asset);
            itr->second.Assets.emplace(asset.AssetId, std::move(asset));
        }while(result->NextRow());
    }

    for (auto& [guildId, house] : _houses)
    {
        house.AssetIdMap.clear();
    
        std::vector<uint32_t> databaseAssetIds;
        databaseAssetIds.reserve(house.Assets.size());
    
        for (auto const& [databaseAssetId, asset] : house.Assets)
            databaseAssetIds.push_back(databaseAssetId);
    
        // Ensure deterministic local IDs.
        std::sort(databaseAssetIds.begin(), databaseAssetIds.end());
    
        uint32_t localAssetId = 1;
    
        for (uint32_t databaseAssetId : databaseAssetIds)
        {
            house.AssetIdMap[localAssetId] = databaseAssetId;
            LOG_INFO("server.loading", "Map: {} -> {}", localAssetId, databaseAssetId);
            ++localAssetId;
        }
    }
    
    LOG_INFO("server.loading", ">> GuildEnclaveMgr loaded {} houses and {} locations", _houses.size(), _locations.size());
}

// =====================================================
// Phase Management
// =====================================================
uint32_t GuildEnclaveMgr::GetPhaseMask(uint32_t guildId) const
{
    const GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return 0;
    
    return house->PhaseMask;
}

bool GuildEnclaveMgr::HasPhase(uint32_t guildId) const
{
    const GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;
    
    return house->PhaseMask > 0;
}

uint32_t GuildEnclaveMgr::GeneratePhaseMask(uint32_t locationId)
{
    GHLocation* location = GetLocation(locationId);
    if (!location)
        return 0;

    for (uint32_t mask = 2; mask <= (1u << 30); mask <<= 1)
    {
        if ((location->InUseBitMask & mask) == 0)
        {
            location->InUseBitMask |= mask;
            return mask;
        }
    }

    return 0;
}

uint32_t GuildEnclaveMgr::CreatePhase(uint32_t guildId, uint32_t locationId)
{
    uint32_t phaseMask = GetPhaseMask(guildId);
    if (phaseMask > 0)
        return phaseMask;

    phaseMask = GeneratePhaseMask(locationId);
    if (!phaseMask)
        return 0;

    return phaseMask;
}

bool GuildEnclaveMgr::RemovePhase(uint32_t guildId)
{
    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house || house->PhaseMask == 0)
        return false;

    GHLocation* location = GetLocation(house->LocationId);
    if (!location)
        return false;

    location->InUseBitMask &= ~house->PhaseMask;
    house->PhaseMask = 0;

    return true;
}

bool GuildEnclaveMgr::EnterPhase(Player* player)
{
    if (!player)
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    const GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;

    GHLocation* location = GetLocation(house->LocationId);
    if (!location)
        return false;

    if (AddMember(player))
    {
        player->TeleportTo(location->MapId, location->X, location->Y, location->Z, location->O);
        player->SetPhaseMask(house->PhaseMask, true);
        return true;
    }

    return false;
}

bool GuildEnclaveMgr::LeavePhase(Player* player)
{
    if (!player)
        return false;

    if (!RemoveMember(player))
        return false;

    player->SetPhaseMask(1, true);
    player->SetRestFlag(REST_FLAG_IN_CITY);
    return true;
}

// =====================================================
// Guild Enclave member 
// =====================================================
bool GuildEnclaveMgr::IsMember(Player* player) const
{
    if (!player)
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    const GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house || house->PhaseMask == 0)
        return false;
    
    return house->Members.find(player->GetGUID().GetCounter()) != house->Members.end();
}

bool GuildEnclaveMgr::AddMember(Player* player)
{
    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;

    if (!HasPhase(guildId))
    {
        int32_t newPhase = CreatePhase(guildId, house->LocationId);
        if (!newPhase)
            return false;
        
        house->PhaseMask = newPhase;
        sGuildEnclaveSpawner.LoadPlacedAssets(guildId);
        
        LOG_INFO("server.loading", "Loaded guild phase {}. ", house->PhaseMask);
    }
    
    house->Members.insert(player->GetGUID().GetCounter());

    LOG_INFO("server.loading", "Addmember to guild phase {}. ", house->PhaseMask);

    return true;
}

bool GuildEnclaveMgr::RemoveMember(Player* player)
{
    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;

    LOG_INFO("server.loading", "Removemember from guild phase");

    house->Members.erase(player->GetGUID().GetCounter());
    if (house->Members.size() == 0)
    {
        sGuildEnclaveSpawner.RemoveAllAssets(guildId);
        RemovePhase (guildId);
        LOG_INFO("server.loading", "Free up guild phase");
    }

    return true;
}

// =====================================================
// Guild Enclave Ownership
// =====================================================
const std::unordered_map<uint32_t, GHGuildEnclave>& GuildEnclaveMgr::GetHouses() const
{
    return _houses;
}

const GHGuildEnclave* GuildEnclaveMgr::GetGuildEnclave(uint32_t guildId) const
{
    auto itr = _houses.find(guildId);
    if (itr == _houses.end())
        return nullptr;

    return &itr->second;
}

GHGuildEnclave* GuildEnclaveMgr::GetGuildEnclave(uint32_t guildId)
{
    auto itr = _houses.find(guildId);
    if (itr == _houses.end())
        return nullptr;

    return &itr->second;
}

bool GuildEnclaveMgr::HasGuildEnclave(uint32_t guildId) const
{
    return _houses.find(guildId) != _houses.end();
}

// =====================================================
// Create and Sell Guild Enclave
// =====================================================
bool GuildEnclaveMgr::CreateGuildEnclave(Player* player, uint32_t guildId, uint32_t ownerGuid, uint32_t locationId)
{
    if (HasGuildEnclave(guildId))
        return false;

    const GHLocation* location = GetLocation(locationId);
    if (!location)
        return false;

    if (!RemoveMoneyFromGuild(guildId, location->Price))
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Not enough money in the Guild bank to purchase. You need - {} in the Guild Bank", GuildEnclaveUtil::GoldToString(location->Price));
        return false;
    }
    
    GHBehaviorFlags faction = player->GetTeamId() == TEAM_ALLIANCE ? GH_FACTION_ALLIANCE : GH_FACTION_HORDE;
    
    CharacterDatabase.Execute("INSERT INTO guildenclave (guildId,ownerGuid,faction,requiredGuildRank,locationId,purchasePrice,purchaseDate) VALUES ({}, {}, {}, 0, {}, {}, (NOW()))",
        guildId, ownerGuid, faction, locationId, location->Price);
    
    GHGuildEnclave house;
    house.GuildId = guildId;
    house.Faction = faction;
    house.OwnerGuid = ownerGuid;
    house.LocationId = locationId;
    house.PhaseMask = 0;
    house.PurchasePrice = location->Price;

    _houses.emplace(guildId, house);

    PurchaseCatalogItem(player, 2);
    
    return true;
}

bool GuildEnclaveMgr::SellGuildEnclave(uint32_t guildId)
{
    auto itr = _houses.find(guildId);
    if (itr == _houses.end())
        return false;

    GHGuildEnclave& house = itr->second;
    uint64_t refund = house.PurchasePrice;

    for (auto const& [assetId, asset] : house.Assets)
        refund += asset.PurchasePrice;

    if (!AddMoneyToGuild(guildId, refund * sGuildEnclaveConfig.GetRefundPercent()))
        return false;

    //ChatHandler(player->GetSession()).PSendSysMessage("Total refunded for sale - {}", GuildEnclaveUtil::GoldToString(refund * sGuildEnclaveConfig.GetRefundPercent()));
    
    sGuildEnclaveSpawner.RemoveAllAssets(guildId);

    CharacterDatabase.Execute("DELETE FROM guildenclave WHERE guildId={}", guildId);
    CharacterDatabase.Execute("DELETE FROM guildenclave_asset WHERE guildId={}", guildId);
    CharacterDatabase.Execute("DELETE FROM guildenclave_spawn WHERE guildId={}", guildId);

    RemovePhase(guildId);

    _houses.erase(itr);

    return true;
}

// =====================================================
// Money Management
// =====================================================
bool GuildEnclaveMgr::SetLocationEnabled(uint32_t locationId, bool enabled)
{
    auto itr = _locations.find(locationId);
    if (itr == _locations.end())
        return false;

    GHLocation& location = itr->second;
    location.Enabled = enabled;

    WorldDatabase.Execute("UPDATE guildenclave_location SET enabled = {} WHERE id = {}", enabled ? 1 : 0, locationId);

    return true;
}

// =====================================================
// Money Management
// =====================================================
bool GuildEnclaveMgr::HasEnoughMoneyInGuild(uint32_t guildId, uint64_t amount)
{
    return true;
    
    if (sGuildEnclaveConfig.IsFree())
        return true;

    if (Guild* guild = sGuildMgr->GetGuildById(guildId))
        return guild->GetTotalBankMoney() >= amount;
    else
        return false;      
}

bool GuildEnclaveMgr::RemoveMoneyFromGuild(uint32_t guildId, uint64_t amount)
{
    return true;
    
    if (sGuildEnclaveConfig.IsFree())
        return true;
    
    if (Guild* guild = sGuildMgr->GetGuildById(guildId))
    {
        if (guild->GetTotalBankMoney() >= amount)
        {
            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
            if (guild->ModifyBankMoney(trans, amount, false)) // false = remove money
            {
                CharacterDatabase.CommitTransaction(trans);
                return true;
            }
        }
    }
    return false;      
}

bool GuildEnclaveMgr::AddMoneyToGuild(uint32_t guildId, uint64_t amount)
{
    return true;
    
    if (sGuildEnclaveConfig.IsFree())
        return true;
    
    if (Guild* guild = sGuildMgr->GetGuildById(guildId))
    {
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        if (guild->ModifyBankMoney(trans, amount, true)) // true = add money
        {
            CharacterDatabase.CommitTransaction(trans);
            return true;
        }
    }
    return false;  
}

// =====================================================
// Teleport
// =====================================================
bool GuildEnclaveMgr::TeleportToGuildEnclave(Player* player)
{
    if (!GuildEnclaveUtil::HasGuildEnclave(player))
        return false;

    return EnterPhase(player);
}

// =====================================================
// Locations
// =====================================================
const GHLocation* GuildEnclaveMgr::GetLocation(uint32_t locationId) const
{
    auto itr = _locations.find(locationId);
    if (itr == _locations.end())
        return nullptr;

    return &itr->second;
}

GHLocation* GuildEnclaveMgr::GetLocation(uint32_t locationId)
{
    auto itr = _locations.find(locationId);
    if (itr == _locations.end())
        return nullptr;

    return &itr->second;
}

const GHLocation* GuildEnclaveMgr::GetGuildLocation(uint32_t guildId) const
{
    const GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return nullptr;

    return GetLocation(house->LocationId);
}

std::vector<const GHLocation*> GuildEnclaveMgr::GetLocations() const
{
    std::vector<const GHLocation*> result;

    for(auto const& [id, location] : _locations)
    {
        if(location.Enabled)
            result.push_back(&location);
    }

    return result;
}

// =====================================================
// Boundary
// =====================================================
bool GuildEnclaveMgr::CheckBoundary(Player* player)
{
    if (IsMember(player) && !GuildEnclaveUtil::IsInsideGuildEnclaveBoundary(player))
    {
        const GHLocation* location = GetGuildLocation(player->GetGuildId());
        player->TeleportTo(location->MapId, location->X, location->Y, location->Z, location->O);
        return false;
    }

    return true;
}

// =====================================================
// Assets
// =====================================================
uint64_t GuildEnclaveMgr::GetTotalAssetCost(Player* player, uint32_t locationId)
{
    uint64_t totalCost = 0;

    QueryResult result = WorldDatabase.Query("SELECT catalogId FROM guildenclave_prebuilt WHERE locationId={}", locationId);
    if (!result)
        return 0;

    do
    {
        Field* fields = result->Fetch();
        uint32_t catalogId = fields[0].Get<uint32_t>();

        const GHCatalog* catalog = sGuildEnclaveCatalogMgr.GetCatalog(catalogId, player->GetTeamId());
        if (!catalog)
            continue;

        totalCost += catalog->Price;
    } while (result->NextRow());

    return totalCost;
}

GHGuildAsset* GuildEnclaveMgr::GetAsset(uint32_t guildId, uint32_t assetId)
{
    auto houseItr = _houses.find(guildId);
    if (houseItr == _houses.end())
        return nullptr;

    auto itr = houseItr->second.Assets.find(assetId);
    if (itr == houseItr->second.Assets.end())
        return nullptr;
    
    return &itr->second;
}

const GHGuildAsset* GuildEnclaveMgr::GetAsset(uint32_t guildId, uint32_t assetId) const
{
    auto houseItr = _houses.find(guildId);
    if (houseItr == _houses.end())
        return nullptr;

    auto itr = houseItr->second.Assets.find(assetId);
    if (itr == houseItr->second.Assets.end())
        return nullptr;
    
    return &itr->second;
}

std::vector<const GHGuildAsset*> GuildEnclaveMgr::GetPurchasedAssets(uint32_t guildId) const
{
    std::vector<const GHGuildAsset*> result;

    auto houseItr = _houses.find(guildId);
    if (houseItr == _houses.end())
        return result;

    for (auto const& entry : houseItr->second.Assets)
    {
        GHGuildAsset const& asset = entry.second;
    
        if (asset.Status == GH_ASSET_PURCHASED || asset.Status == GH_ASSET_PLACED || asset.Status == GH_ASSET_STORED)
            result.push_back(&asset);
    }

    return result;
}

uint32_t GuildEnclaveMgr::ResolveAssetId(uint32_t guildId, uint32_t localAssetId) const
{
    const GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return 0;

    auto itr = house->AssetIdMap.find(localAssetId);
    if (itr == house->AssetIdMap.end())
        return 0;

    return itr->second;
}

bool GuildEnclaveMgr::PlaceAsset(Player* player, uint32_t localAssetId, bool useAssetLocation)
{
    if (!IsMember(player) && !GuildEnclaveUtil::CanManageGuildEnclave(player))
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    uint32_t databaseAssetId = ResolveAssetId(guildId, localAssetId);
    if (!databaseAssetId)
        return false;

    GHGuildAsset* asset = GetAsset(guildId, databaseAssetId);
    if (!asset)
        return false;

    if (!useAssetLocation)
    {
        float z = player->GetPositionZ();
        player->UpdateGroundPositionZ(player->GetPositionX(), player->GetPositionY(), z);
        asset->X = player->GetPositionX();
        asset->Y = player->GetPositionY();
        asset->Z = z;
        asset->O = player->GetOrientation();
//        asset->w = 0;
    }

    if (!sGuildEnclaveSpawner.SpawnAsset(guildId, asset->AssetId, asset->CatalogId, asset->X, asset->Y, asset->Z, asset->O, asset->w))
        return false;

    asset->Status = GH_ASSET_PLACED;

    CharacterDatabase.Execute("UPDATE guildenclave_asset SET status={}, positionX={}, positionY={}, positionZ={}, orientation={}, wander={} WHERE assetId={} AND guildId={}",
        asset->Status, asset->X, asset->Y, asset->Z, asset->O, asset->w, asset->AssetId, guildId);
    return true;
}

bool GuildEnclaveMgr::StoreAsset(Player* player, uint32_t localAssetId)
{
    if (!IsMember(player) && !GuildEnclaveUtil::CanManageGuildEnclave(player))
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    uint32_t databaseAssetId = ResolveAssetId(guildId, localAssetId);
    if (!databaseAssetId)
        return false;

    GHGuildAsset* asset = GetAsset(guildId, databaseAssetId);
    if (!asset)
        return false;

    if (!sGuildEnclaveSpawner.RemoveAsset(guildId, databaseAssetId))
        return false;

    asset->Status = GH_ASSET_STORED;

    CharacterDatabase.Execute("UPDATE guildenclave_asset SET status={} WHERE guildId={} AND assetId={}",
        asset->Status, guildId, asset->AssetId);

    return true;
}

bool GuildEnclaveMgr::MoveAsset(Player* player, uint32_t localAssetId)
{
    if(StoreAsset(player, localAssetId))
        return PlaceAsset(player, localAssetId, false);

    return false;
}

bool GuildEnclaveMgr::SellAsset(Player* player, uint32_t localAssetId)
{
    LOG_INFO("server.loading", "SellAsset ResolveAssetId");
    uint32_t databaseAssetId = ResolveAssetId(player->GetGuildId(), localAssetId);
    if (!databaseAssetId)
        return false;

    LOG_INFO("server.loading", "SellAsset RemoveAsset");
    return RemoveAsset(player, databaseAssetId, true);
}

// =====================================================
// Builds
// =====================================================
uint32_t GuildEnclaveMgr::AddAsset(Player* player, uint32_t catalogId, float X, float Y, float Z, float O, int32_t W, bool charge)
{
    if (!GuildEnclaveUtil::IsGuildRank(player))
        return 0;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return 0;

    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return 0;

    const GHCatalog* catalog = sGuildEnclaveCatalogMgr.GetCatalog(catalogId, player->GetTeamId());
    if (!catalog || !catalog->Enabled)
        return 0;
    
    ChatHandler(player->GetSession()).PSendSysMessage("Catalog -> {}.", catalog->Name);
    
    if (charge && !RemoveMoneyFromGuild(guildId, catalog->Price))
    {
            ChatHandler(player->GetSession()).PSendSysMessage("Not enough money in the Guild bank to purchase.");
            return 0;
    }
    
    CharacterDatabase.Execute("INSERT INTO guildenclave_asset (assetId,guildId,catalogId,purchasePrice,status,positionX,positionY,positionZ,orientation,wander,createdBy) VALUES ({},{},{},{},{},{},{},{},{},{},{})",
        _nextAssetId, guildId, catalogId, charge ? catalog->Price : 0, GH_ASSET_PURCHASED, X, Y, Z, O, W, player->GetGUID().GetCounter());

    uint32_t assetId = _nextAssetId++;

    GHGuildAsset asset;
    asset.AssetId = assetId;
    asset.GuildId = guildId;
    asset.CatalogId = catalogId;
    asset.PurchasePrice = charge ? catalog->Price : 0;
    asset.Status = GH_ASSET_PURCHASED;
    asset.X = X;
    asset.Y = Y;
    asset.Z = Z;
    asset.O = O;
    asset.w = W;

    house->Assets.emplace(assetId, std::move(asset));

    uint32_t localAssetId = 1;
    while (house->AssetIdMap.contains(localAssetId))
        ++localAssetId;
    house->AssetIdMap[localAssetId] = assetId;
    LOG_INFO("server.loading", "Map: {} -> {}", localAssetId, assetId);
    
    return localAssetId;  
}

uint32_t GuildEnclaveMgr::AddAsset(Player* player, uint32_t catalogId, bool charge)
{
    return AddAsset(player, catalogId, 0, 0, 0, 0, 0, charge);
}

bool GuildEnclaveMgr::RemoveAsset(Player* player, uint32_t assetId, bool refund)
{
    if (!GuildEnclaveUtil::IsGuildRank(player))
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;

    GHGuildAsset* asset = GetAsset(guildId, assetId);
    if (!asset || asset->CatalogId < 100)
        return false;

    uint32_t localAssetId = 0;
    for (auto const& [localId, databaseId] : house->AssetIdMap)
    {
        if (databaseId == assetId)
        {
            localAssetId = localId;
            break;
        }
    }
    if (!localAssetId)
        return false;
    
    if (refund)
    {
        if(AddMoneyToGuild(guildId, asset->PurchasePrice * sGuildEnclaveConfig.GetRefundPercent()))
            ChatHandler(player->GetSession()).PSendSysMessage("Total refunded for sale - {}", GuildEnclaveUtil::GoldToString(asset->PurchasePrice * sGuildEnclaveConfig.GetRefundPercent()));
    }

    sGuildEnclaveSpawner.RemoveAsset(guildId, assetId);

    CharacterDatabase.Execute("DELETE FROM guildenclave_asset WHERE guildId={} AND assetId={}", guildId, assetId);

    auto itr = house->Assets.find(assetId);
    if (itr != house->Assets.end())
        house->Assets.erase(itr);

    house->AssetIdMap.erase(localAssetId);
    std::unordered_map<uint32_t, uint32_t> newAssetIdMap;
    for (auto const& [oldLocalId, databaseId] : house->AssetIdMap)
    {
        uint32_t newLocalId = oldLocalId;
        if (oldLocalId > localAssetId)
            newLocalId--;
        newAssetIdMap[newLocalId] = databaseId;
    }
    house->AssetIdMap = std::move(newAssetIdMap);
    
    return true;
}

bool GuildEnclaveMgr::LoadBuild(Player* player)
{
    if (!GuildEnclaveUtil::IsGuildRank(player))
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;
    
    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;

    if(QueryResult result = WorldDatabase.Query("SELECT catalogId,X,Y,Z,O,W FROM guildenclave_prebuilt WHERE locationId={}", house->LocationId))
    {
        ClearBuild(player);

        do
        {
            Field* fields = result->Fetch();
            uint32_t localAssetId = AddAsset(player, fields[0].Get<uint32>(), fields[1].Get<float>(), fields[2].Get<float>(), fields[3].Get<float>(), fields[4].Get<float>(), fields[5].Get<float>(), false);
            PlaceAsset(player, localAssetId, true);
        }while(result->NextRow());
    }

    ChatHandler(player->GetSession()).PSendSysMessage("All assets have been loaded for build {}", house->LocationId);
    return true;
}

bool GuildEnclaveMgr::SaveBuild(Player* player)
{
    if (!GuildEnclaveUtil::IsGuildRank(player))
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;
    
    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;
    
    WorldDatabase.Query("DELETE FROM guildenclave_prebuilt WHERE locationId={}", house->LocationId);
    
    for (auto const& [assetId, asset] : house->Assets)
    {
        if (GuildEnclaveUtil::HasFlag(asset.Status, GH_ASSET_PLACED) && asset.CatalogId >= 100)
            WorldDatabase.Query("INSERT INTO guildenclave_prebuilt (locationId,catalogId,X,Y,Z,O,W) VALUES({},{},{},{},{},{},{})", house->LocationId, asset.CatalogId, asset.X, asset.Y, asset.Z, asset.O, asset.w);
    }
    
    ChatHandler(player->GetSession()).PSendSysMessage("All assets have been saved except Salesman to build {}", house->LocationId);
    return true;
}

bool GuildEnclaveMgr::ClearBuild(Player* player)
{
    if (!GuildEnclaveUtil::IsGuildRank(player))
        return false;

     uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;

    std::vector<uint32_t> assetIds;
    for (auto const& [assetId, asset] : house->Assets)
    {
        if (asset.CatalogId >= 100)
            assetIds.push_back(assetId);
    }

    for (uint32_t assetId : assetIds)
        if (!RemoveAsset(player, assetId, false))
            return false;

    ChatHandler(player->GetSession()).PSendSysMessage("All assets have been removed except the Salesman");
    return true;
}

bool GuildEnclaveMgr::AddToBuild(Player* player, uint32_t catalogId)
{
    if (!GuildEnclaveUtil::CanManageGuildEnclave(player))
        return false;

    uint32_t assetId = AddAsset(player, catalogId, false);
    if (assetId == 0)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Unable to add asset.");
        return false;
    }
    
    if (!PlaceAsset(player, assetId, false))
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Asset added, but unable to place, try the command .ge asset place {}", assetId);
        return false;
    }

    return true;    
}

bool GuildEnclaveMgr::RemoveFromBuild(Player* player, uint32_t assetId)
{
    uint32_t databaseAssetId = ResolveAssetId(player->GetGuildId(), assetId);
    if (!databaseAssetId)
        return false;
    
    return RemoveAsset(player, databaseAssetId, false);
}

bool GuildEnclaveMgr::WanderAsset(Player* player, uint32_t localAssetId, uint32_t distance)
{
    if (!IsMember(player) && !GuildEnclaveUtil::CanManageGuildEnclave(player))
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    uint32_t databaseAssetId = ResolveAssetId(player->GetGuildId(), localAssetId);
    if (!databaseAssetId)
        return false;

    GHGuildAsset* asset = GetAsset(guildId, databaseAssetId);
    if (!asset)
        return false;

    asset->w = distance;
    CharacterDatabase.Execute("UPDATE guildenclave_asset SET wander={} WHERE assetId={} AND guildId={}", distance, asset->AssetId, guildId);
    if (asset->Status == GH_ASSET_PLACED)
    {
        if (!sGuildEnclaveSpawner.RemoveAsset(guildId, asset->AssetId))
            return false;
        sGuildEnclaveSpawner.SpawnAsset(guildId, asset->AssetId, asset->CatalogId, asset->X, asset->Y, asset->Z, asset->O, asset->w);
    }

    return true;
}

// =====================================================
// Catalog Purchase
// =====================================================
bool GuildEnclaveMgr::PurchaseCatalogItem(Player* player, uint32_t catalogId)
{
    uint32_t assetId = AddAsset(player, catalogId, true);
    if (assetId == 0)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Unable to add asset.");
        return false;
    }
    
    return true;
}
