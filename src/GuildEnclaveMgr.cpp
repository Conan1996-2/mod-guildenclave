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
            house.Team =  fields[2].Get<uint8_t>();
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

    if(QueryResult result = CharacterDatabase.Query("SELECT spawnId,guildId,assetId,phaseMask,spawnGuid,spawnType,mapId,x,y,z,o,w FROM guildenclave_spawn"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 guildId = fields[1].Get<uint32>();

            auto itr = _houses.find(guildId);
            if(itr == _houses.end())
                continue;

            GHGuildSpawn spawn;
            spawn.SpawnId = fields[0].Get<uint32>();
            spawn.GuildId = fields[1].Get<uint32>();
            spawn.AssetId = fields[2].Get<uint32>();
            spawn.PhaseMask = fields[3].Get<uint32>();
            spawn.SpawnGuid = fields[4].Get<uint32>();
            spawn.SpawnType = fields[5].Get<uint8>();

            spawn.MapId = fields[6].Get<uint32>();
            spawn.X = fields[7].Get<float>();
            spawn.Y = fields[8].Get<float>();
            spawn.Z = fields[9].Get<float>();
            spawn.O = fields[10].Get<float>();
            spawn.w = fields[11].Get<float>();

            itr->second.Spawns.push_back(spawn);
        }while(result->NextRow());
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
    
    CharacterDatabase.Execute("INSERT INTO guildenclave (guildId,ownerGuid,faction,requiredGuildRank,locationId,purchasePrice,purchaseDate) VALUES ({}, {}, {}, 0, {}, {}, (NOW()))",
        guildId, ownerGuid, static_cast<uint8>(player->GetTeamId()), locationId, location->Price);
    
    GHGuildEnclave house;
    house.GuildId = guildId;
    house.Team = static_cast<uint8>(player->GetTeamId());
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
    if (!IsMember(player) && !GuildEnclaveUtil::IsInsideGuildEnclaveBoundry(player))
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

bool GuildEnclaveMgr::PlaceAsset(Player* player, uint32_t assetId)
{
    if (!IsMember(player) && !GuildEnclaveUtil::CanManageGuildEnclave(player))
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildAsset* asset = GetAsset(guildId, assetId);
    if (!asset)
        return false;

    float x = player->GetPositionX();
    float y = player->GetPositionY();
    float z = player->GetPositionZ();
    int16_t w = asset->w;

    player->UpdateGroundPositionZ(x, y, z);

    if (!sGuildEnclaveSpawner.SpawnAsset(guildId, asset->AssetId, asset->CatalogId, x, y, z, player->GetOrientation(), w))
        return false;

    asset->Status = GH_ASSET_PLACED;
    asset->X = x;
    asset->Y = y;
    asset->Z = z;
    asset->O = player->GetOrientation();
    asset->w = w;

    CharacterDatabase.Execute("UPDATE guildenclave_asset SET status={}, positionX={}, positionY={}, positionZ={}, orientation={}, wander={} WHERE assetId={} AND guildId={}",
        asset->Status, x, y, z, asset->O, w, assetId, guildId);
    return true;
}

bool GuildEnclaveMgr::StoreAsset(Player* player, uint32_t assetId)
{
    if (!IsMember(player) && !GuildEnclaveUtil::CanManageGuildEnclave(player))
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildAsset* asset = GetAsset(guildId, assetId);
    if (!asset)
        return false;

    if (!sGuildEnclaveSpawner.RemoveAsset(guildId, assetId))
        return false;

    asset->Status = GH_ASSET_STORED;

    CharacterDatabase.Execute("UPDATE guildenclave_asset SET status={} WHERE guildId={} AND assetId={}",
        asset->Status, guildId, assetId);

    return true;
}

bool GuildEnclaveMgr::MoveAsset(Player* player, uint32_t assetId)
{
    if(StoreAsset(player, assetId))
        return PlaceAsset(player, assetId);

    return false;
}

bool GuildEnclaveMgr::SellAsset(Player* player, uint32_t assetId)
{
    if (!IsMember(player) && !GuildEnclaveUtil::CanManageGuildEnclave(player))
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;

    GHGuildAsset* asset = GetAsset(guildId, assetId);
    if (!asset)
        return false;

    if (!sGuildEnclaveSpawner.RemoveAsset(guildId, assetId))
        return false;

    if (!AddMoneyToGuild(guildId, asset->PurchasePrice * sGuildEnclaveConfig.GetRefundPercent()))
        return false;

    ChatHandler(player->GetSession()).PSendSysMessage("Total refunded for sale - {}", GuildEnclaveUtil::GoldToString(asset->PurchasePrice * sGuildEnclaveConfig.GetRefundPercent()));
    
    CharacterDatabase.Execute("DELETE FROM guildenclave_asset WHERE guildId={} AND assetId={}", guildId, assetId);

    auto itr = house->Assets.find(assetId);
    if (itr != house->Assets.end())
        house->Assets.erase(itr);

    return true;
}

bool GuildEnclaveMgr::WanderAsset(Player* player, uint32_t assetId, uint32_t distance)
{
    return true;
}

// =====================================================
// Catalog Purchase
// =====================================================
bool GuildEnclaveMgr::PurchaseCatalogItem(Player* player, uint32_t catalogId)
{
    if (!player)
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildEnclave* house = GetGuildEnclave(guildId);
    if (!house)
        return false;

    if (!GuildEnclaveUtil::IsGuildRank(player))
        return false;

    const GHCatalog* catalog = sGuildEnclaveCatalogMgr.GetCatalog(catalogId);
    if (!catalog || !catalog->Enabled)
        return false;

    if (!RemoveMoneyFromGuild(guildId, catalog->Price))
    {
            ChatHandler(player->GetSession()).PSendSysMessage("Not enough money in the Guild bank to purchase.");
            return false;
    }
    
    CharacterDatabase.Execute("INSERT INTO guildenclave_asset (assetId,guildId,catalogId,purchasePrice,status,positionX,positionY,positionZ,orientation,wander,createdBy) VALUES ({},{},{},{},{},0,0,0,0,{},{})",
        _nextAssetId, guildId, catalogId, catalog->Price, GH_ASSET_PURCHASED, 0, player->GetGUID().GetCounter());

    uint32_t assetId = _nextAssetId++;

    GHGuildAsset asset;
    asset.AssetId = assetId;
    asset.GuildId = guildId;
    asset.CatalogId = catalogId;
    asset.PurchasePrice = catalog->Price;
    asset.Status = GH_ASSET_PURCHASED;
    asset.X = 0.0f;
    asset.Y = 0.0f;
    asset.Z = 0.0f;
    asset.O = 0.0f;
    asset.w = 0;

    house->Assets.emplace(assetId, std::move(asset));

    return true;
}
