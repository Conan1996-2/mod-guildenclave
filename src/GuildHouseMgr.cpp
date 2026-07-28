#include "ScriptMgr.h"

#include "GuildHouseMgr.h"

#include "GuildHouseConfig.h"
#include "GuildHousePhaseMgr.h"
#include "GuildHouseSpawner.h"
#include "GuildHouseCatalogMgr.h"
#include "GuildHouseDefines.h"

#include "DatabaseEnv.h"
#include "QueryResult.h"

#include "Player.h"
#include "Chat.h"
#include "Log.h"
#include "GuildMgr.h"

#include <algorithm>
#include <cstdint>

GuildHouseMgr& GuildHouseMgr::Instance()
{
    static GuildHouseMgr instance;
    return instance;
}

// =====================================================
// Phase Management
// =====================================================
bool GuildHouseMgr::HasPhase(uint32_t guildId) const
{
    return sGuildHousePhaseMgr.HasPhase(guildId);
}

const GHPhaseRecord* GuildHouseMgr::GetPhase(uint32_t guildId) const
{
    return sGuildHousePhaseMgr.GetPhase(guildId);
}

uint32_t GuildHouseMgr::GetPhaseMask(uint32_t guildId) const
{
    return sGuildHousePhaseMgr.GetPhaseMask(guildId);
}

uint32_t GuildHouseMgr::CreatePhase(uint32_t guildId, uint32_t locationId)
{
    return sGuildHousePhaseMgr.CreatePhase(guildId, locationId);
}

bool GuildHouseMgr::EnterPhase(Player* player, uint32_t guildId)
{
    return sGuildHousePhaseMgr.EnterPhase(player, guildId);
}

bool GuildHouseMgr::RemovePhase(uint32_t guildId)
{
    return sGuildHousePhaseMgr.RemovePhase(guildId);
}

// =====================================================
// Guild House Ownership
// =====================================================
bool GuildHouseMgr::HasGuildHouse(uint32_t guildId) const
{
    return _houses.find(guildId) != _houses.end();
}

const GHGuildHouse* GuildHouseMgr::GetGuildHouse(uint32_t guildId) const
{
    auto itr = _houses.find(guildId);
    if (itr == _houses.end())
        return nullptr;

    return &itr->second;
}

const std::unordered_map<uint32_t, GHGuildHouse>& GuildHouseMgr::GetHouses() const
{
    return _houses;
}

GHGuildHouse* GuildHouseMgr::GetGuildHouse(uint32_t guildId)
{
    auto itr = _houses.find(guildId);
    if (itr == _houses.end())
        return nullptr;

    return &itr->second;
}

const GHLocation* GuildHouseMgr::GetGuildLocation(uint32_t guildId) const
{
    const GHGuildHouse* house = GetGuildHouse(guildId);
    if (!house)
        return nullptr;

    return GetLocation(house->LocationId);
}

// =====================================================
// Create Guild House
// =====================================================
bool GuildHouseMgr::CreateGuildHouse(Player* player, uint32_t guildId, uint32_t ownerGuid, uint32_t locationId)
{
    if (HasGuildHouse(guildId))
        return false;

    const GHLocation* location = GetLocation(locationId);
    if (!location)
        return false;

    if (!RemoveMoneyFromGuild(guildId, location->Price))
    {
            ChatHandler(player->GetSession()).PSendSysMessage("Not enough money in the Guild bank to purchase.");
            return false;
    }
    
    CharacterDatabase.Execute("INSERT INTO guildhouse (guildId,ownerGuid,requiredGuildRank,locationId,purchasePrice,purchaseDate) VALUES ({}, {}, 0, {}, {}, (NOW()))",
        guildId, ownerGuid, locationId, location->Price);

    uint32_t phaseMask = CreatePhase( guildId, locationId);
    if (!phaseMask)
    {
        CharacterDatabase.Execute("DELETE FROM guildhouse WHERE guildId={}", guildId);
        return false;
    }

    GHGuildHouse house;
    house.GuildId = guildId;
    house.OwnerGuid = ownerGuid;
    house.LocationId = locationId;
    house.PhaseMask = phaseMask;
    house.PurchasePrice = location->Price;

    _houses.emplace(guildId, house);

    return true;
}

// =====================================================
// Sell Guild House
// =====================================================
bool GuildHouseMgr::SellGuildHouse(uint32_t guildId)
{
    auto itr = _houses.find(guildId);
    if (itr == _houses.end())
        return false;

    GHGuildHouse& house = itr->second;

    uint64_t refund = house.PurchasePrice;

    for (auto const& [assetId, asset] : house.Assets)
        refund += asset.PurchasePrice;

    if (!AddMoneyToGuild(guildId, refund * sGuildHouseConfig.GetRefundPercent() / 100))
        return false;

    sGuildHouseSpawner.RemoveAllAssets(guildId);

    CharacterDatabase.Execute("DELETE FROM guildhouse WHERE guildId={}", guildId);
    CharacterDatabase.Execute("DELETE FROM guildhouse_asset WHERE guildId={}", guildId);
    CharacterDatabase.Execute("DELETE FROM guildhouse_spawn WHERE guildId={}", guildId);

    RemovePhase(guildId);

    _houses.erase(itr);

    return true;
}

// =====================================================
// Money Management
// =====================================================
bool GuildHouseMgr::HasEnoughMoneyInGuild(uint32_t guildId, uint64_t amount)
{
    return true;
    
    if (Guild* guild = sGuildMgr->GetGuildById(guildId))
        return guild->GetTotalBankMoney() >= amount;
    else
        return false;      
}

bool GuildHouseMgr::RemoveMoneyFromGuild(uint32_t guildId, uint64_t amount)
{
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

bool GuildHouseMgr::AddMoneyToGuild(uint32_t guildId, uint64_t amount)
{
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
bool GuildHouseMgr::TeleportToGuildHouse(Player* player)
{
    if (!player)
        return false;

    uint32_t guildId =player->GetGuildId();
    if (!guildId)
        return false;

    const GHGuildHouse* house = GetGuildHouse(guildId);
    if (!house)
        return false;

    const GHLocation* location = GetGuildLocation(guildId);
    if (!location)
        return false;

    if (!HasPhase(guildId))
    {
        if (!CreatePhase(guildId, house->LocationId))
            return false;
    }

    return EnterPhase(player, guildId);
}

// =====================================================
// Locations
// =====================================================
const GHLocation* GuildHouseMgr::GetLocation(uint32_t locationId) const
{
    auto itr = _locations.find(locationId);
    if (itr == _locations.end())
        return nullptr;

    return &itr->second;
}

std::vector<const GHLocation*> GuildHouseMgr::GetLocations() const
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
bool GuildHouseMgr::IsInsideGuildHouseBoundary(uint32_t guildId, float x, float y) const
{
    const GHLocation* location = GetGuildLocation(guildId);
    if(!location)
        return false;

    return x >= location->MinX && x <= location->MaxX && y >= location->MinY && y <= location->MaxY;
}

// =====================================================
// Load
// =====================================================
void GuildHouseMgr::Load()
{
    LOG_INFO("server.loading", "Loading GuildHouseMgr");    

    _houses.clear();
    _locations.clear();

    // -------------------------------------------------
    // Get asset maximum database number
    // -------------------------------------------------
    if(QueryResult result = CharacterDatabase.Query("SELECT MAX(assetId) FROM guildhouse_asset"))
    {
        Field* fields = result->Fetch();
    
        uint32_t maxAssetId = fields[0].Get<uint32>();
        _nextAssetId = maxAssetId + 1;
    }
    
    // -------------------------------------------------
    // Locations
    // -------------------------------------------------
    LOG_INFO("server.loading", "Loading GuildHouseMgr::guildhouse_locations");    

    if(QueryResult result = WorldDatabase.Query("SELECT id,name,mapId,positionX,positionY,positionZ,orientation,minX,maxX,minY,maxY,price,enabled FROM guildhouse_locations"))
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

    // -------------------------------------------------
    // Guild Houses
    // -------------------------------------------------
    LOG_INFO("server.loading", "Loading GuildHouseMgr::guildhouse");    

    if(QueryResult result = CharacterDatabase.Query("SELECT guildId,ownerGuid,requiredGuildRank,locationId,purchasePrice FROM guildhouse"))
    {
        do
        {
            Field* fields = result->Fetch();

            GHGuildHouse house;
            house.GuildId = fields[0].Get<uint32>();
            house.OwnerGuid = fields[1].Get<uint32>();
            house.RequiredGuildRank = fields[2].Get<uint8_t>();
            house.LocationId = fields[3].Get<uint32>();
            house.PurchasePrice = fields[4].Get<uint64_t>();
            house.PhaseMask = 0;

            _houses.emplace(house.GuildId, house);

        }while(result->NextRow());
    }

    // -------------------------------------------------
    // Load phases
    // -------------------------------------------------
    LOG_INFO("server.loading", "Loading GuildHouseMgr::phases");    

    sGuildHousePhaseMgr.Load();

    for(auto& [guildId, house] : _houses)
        house.PhaseMask = GetPhaseMask(guildId);

    // -------------------------------------------------
    // Assets
    // unused: status, createdBy, enabled, createdDate
    // ------------------------------------------------- 
    LOG_INFO("server.loading", "Loading GuildHouseMgr::asset");    

    if(QueryResult result = CharacterDatabase.Query("SELECT assetId,guildId,catalogId,purchasePrice,status,positionX,positionY,positionZ,orientation FROM guildhouse_asset"))
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

            //itr->second.Assets.push_back(asset);
            itr->second.Assets.emplace(asset.AssetId, std::move(asset));
        }while(result->NextRow());
    }

    // -------------------------------------------------
    // Spawns
    // -------------------------------------------------
    LOG_INFO("server.loading", "Loading GuildHouseMgr::spawns");    

    if(QueryResult result = CharacterDatabase.Query("SELECT spawnId,guildId,assetId,phaseMask,spawnGuid,spawnType,mapId,x,y,z,o FROM guildhouse_spawn"))
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

            itr->second.Spawns.push_back(spawn);
        }while(result->NextRow());
    }

    LOG_INFO("server.loading", "GuildHouseMgr loaded {} houses and {} locations", _houses.size(), _locations.size());
}

GHGuildAsset* GuildHouseMgr::GetAsset(uint32_t guildId, uint32_t assetId)
{
    auto houseItr = _houses.find(guildId);
    if (houseItr == _houses.end())
        return nullptr;

    auto itr = houseItr->second.Assets.find(assetId);
    if (itr == houseItr->second.Assets.end())
        return nullptr;
    
    return &itr->second;
}

const GHGuildAsset* GuildHouseMgr::GetAsset(uint32_t guildId, uint32_t assetId) const
{
    auto houseItr = _houses.find(guildId);
    if (houseItr == _houses.end())
        return nullptr;

    auto itr = houseItr->second.Assets.find(assetId);
    if (itr == houseItr->second.Assets.end())
        return nullptr;
    
    return &itr->second;

}

std::vector<const GHGuildAsset*> GuildHouseMgr::GetPurchasedAssets(uint32_t guildId) const
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

bool GuildHouseMgr::PlaceAsset(Player* player, uint32_t assetId)
{
    if (!player)
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildHouse* house = GetGuildHouse(guildId);
    if (!house)
        return false;

    if (!sGuildHousePhaseMgr.IsMember(player))
        return false;

    GHGuildAsset* asset = GetAsset(guildId, assetId);
    if (!asset)
        return false;

    LOG_INFO("server.loading", "GuildHouseMgr PlaceAsset spawning asset {}", assetId);

    if (!sGuildHouseSpawner.SpawnAsset(guildId, asset->AssetId, asset->CatalogId, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()))
        return false;

    asset->Status = GH_ASSET_PLACED;
    asset->X = player->GetPositionX();
    asset->Y = player->GetPositionY();
    asset->Z = player->GetPositionZ();
    asset->O = player->GetOrientation();

    CharacterDatabase.Execute("UPDATE guildhouse_asset SET status={}, positionX={}, positionY={}, positionZ={}, orientation={} WHERE assetId={} AND guildId={}",
        asset->Status, asset->X, asset->Y, asset->Z, asset->O, assetId, guildId);
    return true;
}

bool GuildHouseMgr::StoreAsset(Player* player, uint32_t assetId)
{
    if (!player)
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    if (!sGuildHousePhaseMgr.IsMember(player))
        return false;

    GHGuildAsset* asset = GetAsset(guildId, assetId);
    if (!asset)
        return false;

    if (!sGuildHouseSpawner.RemoveAsset(guildId, assetId))
        return false;

    asset->Status = GH_ASSET_STORED;

    CharacterDatabase.Execute("UPDATE guildhouse_asset SET status={} WHERE guildId={} AND assetId={}",
        asset->Status, guildId, assetId);

    return true;
}

bool GuildHouseMgr::MoveAsset(Player* player, uint32_t assetId)
{
    if(StoreAsset(player, assetId))
        return PlaceAsset(player, assetId);

    return false;
}

bool GuildHouseMgr::SellAsset(Player* player, uint32_t assetId)
{
    if (!player)
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    //if (!sGuildHousePhaseMgr.IsMember(player))
    //    return false;

    GHGuildHouse* house = GetGuildHouse(guildId);
    if (!house)
        return false;

    GHGuildAsset* asset = GetAsset(guildId, assetId);
    if (!asset)
        return false;

    if (!sGuildHouseSpawner.RemoveAsset(guildId, assetId))
        return false;

    if (!AddMoneyToGuild(guildId, asset->PurchasePrice * sGuildHouseConfig.GetRefundPercent() / 100))
        return false;

    CharacterDatabase.Execute("DELETE FROM guildhouse_asset WHERE guildId={} AND assetId={}", guildId, assetId);

    auto itr = house->Assets.find(assetId);
    if (itr != house->Assets.end())
        house->Assets.erase(itr);

    return true;
}

// =====================================================
// Catalog Purchase
// =====================================================
bool GuildHouseMgr::PurchaseCatalogItem(Player* player, uint32_t catalogId)
{
    if (!player)
        return false;

    uint32_t guildId = player->GetGuildId();
    if (!guildId)
        return false;

    GHGuildHouse* house = GetGuildHouse(guildId);
    if (!house)
        return false;

    if (!GuildHouseUtil::IsGuildRank(player))
        return false;

    const GHCatalog* catalog = sGuildHouseCatalogMgr.GetCatalog(catalogId);
    if (!catalog || !catalog->Enabled)
        return false;

    if (!RemoveMoneyFromGuild(guildId, catalog->Price))
    {
            ChatHandler(player->GetSession()).PSendSysMessage("Not enough money in the Guild bank to purchase.");
            return false;
    }
    
    CharacterDatabase.Execute("INSERT INTO guildhouse_asset (assetId,guildId,catalogId,purchasePrice,status,positionX,positionY,positionZ,orientation,createdBy) VALUES ({},{},{},{},{},0,0,0,0,{})",
        _nextAssetId, guildId, catalogId, catalog->Price, GH_ASSET_PURCHASED, player->GetGUID().GetCounter());

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

    house->Assets.emplace(assetId, std::move(asset));

    return true;
}

// =====================================================
// Does the Salesman exist
// =====================================================
bool GuildHouseMgr::HasSalesman(uint32_t guildId) const
{
    auto houseItr = _houses.find(guildId);
    if (houseItr == _houses.end())
        return false;

    for (GHGuildSpawn const& spawn : houseItr->second.Spawns)
    {
        if (spawn.AssetId == 0)
            return true;
    }

    return false;
}

// =====================================================
// Create the Salesman
// =====================================================
bool GuildHouseMgr::CreatePermanentSalesman(Player* player, uint32_t entry)
{
    if(!player)
        return false;

    uint32_t guildId = player->GetGuildId();
    if(HasSalesman(guildId))
        return false;

    uint32_t phaseMask = GetPhaseMask(guildId);
    if(!phaseMask)
        return false;

    return (sGuildHouseSpawner.SpawnCreature(guildId, 0, phaseMask, player->GetMapId(), entry, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()));
}

// =====================================================
// Script Registration
// =====================================================
class GuildHouseWorldScript : public WorldScript
{
public:

    GuildHouseWorldScript()
        : WorldScript("GuildHouseWorldScript")
    {
    }

    void OnStartup() override
    {
        sGuildHouseCatalogMgr.Load();
        sGuildHousePhaseMgr.Load();
        sGuildHouseMgr.Load();
        sGuildHouseSpawner.LoadPlacedAssets();
    }
};

void AddSC_GuildHouseMgr()
{
    new GuildHouseWorldScript();
}
