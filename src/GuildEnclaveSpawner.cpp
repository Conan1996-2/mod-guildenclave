#include "GuildEnclaveSpawner.h"

#include "GuildEnclaveConfig.h"
#include "GuildEnclaveTypes.h"
#include "GuildEnclaveMgr.h"
//#include "GuildEnclavePhaseMgr.h"
#include "GuildEnclaveCatalogMgr.h"

#include "MapMgr.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "GameObject.h"
#include "Transport.h"
#include "GuildMgr.h"

#include <sstream>

GuildEnclaveSpawner& GuildEnclaveSpawner::Instance()
{
    static GuildEnclaveSpawner instance;
    return instance;
}

// =====================================================
// Existing spawn
// =====================================================
bool GuildEnclaveSpawner::HasExistingSpawn(uint32_t guildId, uint32_t assetId)
{
    GHGuildEnclave const* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);

    if (!house)
        return false;

    for (GHGuildSpawn const& spawn : house->Spawns)
    {
        if (spawn.AssetId == assetId && spawn.SpawnGuid != 0)
            return true;
    }

    return false;
}

// =====================================================
// Spawn Asset
// =====================================================
bool GuildEnclaveSpawner::SpawnAsset(uint32_t guildId, uint32_t assetId, uint32_t catalogId, float x, float y, float z, float o, int w)
{
    if(HasExistingSpawn(guildId, assetId))
        return false;

    GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
    if (!house)
        return false;

    GHLocation* location = sGuildEnclaveMgr.GetLocation(house->LocationId);
    if (!location)
        return false;
    
    const GHCatalog* catalog = sGuildEnclaveCatalogMgr.GetCatalog(catalogId, static_cast<TeamId>(house->faction == GH_FACTION_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE));
    if(!catalog)
        return false;

    for(auto const& component : catalog->Components)
    {
        if (!GuildEnclaveUtil::HasFlag(component.BehaviorFlags, GH_FACTION_NEUTRAL) || GuildEnclaveUtil::HasFlag(component.BehaviorFlags, house->faction))
                continue;

        float sinO = std::sin(o);
        float cosO = std::cos(o);
    
        float cx = x + component.XOffset * cosO - component.YOffset * sinO;
        float cy = y + component.XOffset * sinO + component.YOffset * cosO;
        float cz = z + component.ZOffset;
        float co = Position::NormalizeOrientation(o + component.OOffset);

        if(GuildEnclaveUtil::HasFlag(component.SpawnFlags, GH_SPAWN_CREATURE))
            SpawnCreature(guildId, assetId, house->PhaseMask, location->MapId, component.Entry, cx, cy, cz, co, w);

        if(GuildEnclaveUtil::HasFlag(component.SpawnFlags, GH_SPAWN_GAMEOBJECT))
            SpawnGameObject(guildId, assetId, house->PhaseMask, location->MapId, component.Entry, cx, cy, cz, co);
    }

    return true;
}

void GuildEnclaveSpawner::LoadPlacedAssets(uint32_t guildId)
{
    const GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
    if (!house)
        return;

    for (auto const& [assetId, asset] : house->Assets)
    {
        if (asset.Status != GH_ASSET_PLACED)
            continue;

        if (asset.CatalogId > 0)
            SpawnAsset(guildId, asset.AssetId, asset.CatalogId, asset.X, asset.Y, asset.Z, asset.O, asset.w);
        else
        {
            GHLocation* location = sGuildEnclaveMgr.GetLocation(house->LocationId);
            if (!location)
                return;
            
            uint32 entry = house->faction == GH_FACTION_ALLIANCE ? 900002 : 900003;
            SpawnCreature(guildId, 0, house->PhaseMask, location->MapId, entry, asset.X, asset.Y, asset.Z, asset.O, 0);
        }
    }
}

// =====================================================
// Creature
// =====================================================
bool GuildEnclaveSpawner::SpawnCreature(uint32_t guildId, uint32_t assetId, uint32_t phaseMask, uint32_t mapId, uint32_t entry, float x, float y, float z, float o, int w)
{
   Map* map = sMapMgr->CreateBaseMap(mapId);
    if (!map)
        return false;
    
    Creature* creature = new Creature();
    if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, phaseMask, entry, 0, x, y, z, o))
    {
        delete creature;
        return false;
    }

    if (sGuildEnclaveConfig.AllWander() && w== 0)
        w = sGuildEnclaveConfig.WanderDistance();

    if (w > 0)
    {
        creature->SetWanderDistance(w);
        creature->SetDefaultMovementType(MovementGeneratorType::RANDOM_MOTION_TYPE);
    }

    creature->SaveToDB(mapId, (1 << map->GetSpawnMode()), phaseMask);
    uint32 spawnId = creature->GetSpawnId();
    creature->CleanupsBeforeDelete();
    delete creature;

    creature = new Creature();
    if (!creature->LoadCreatureFromDB(spawnId, map))
    {
        delete creature;
        return false;
    }

    sObjectMgr->AddCreatureToGrid(spawnId, sObjectMgr->GetCreatureData(spawnId));

    CharacterDatabase.Execute("INSERT INTO guildenclave_spawn (guildId,assetId,spawnGuid,spawnType,mapId,phaseMask,x,y,z,o,w) "
        "VALUES ({},{},{},{},{},{},{},{},{},{},{})", guildId, assetId, spawnId, GH_SPAWN_CREATURE, mapId, phaseMask, x, y, z, o, w);

    GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
    if (house)
    {
        GHGuildSpawn spawn;
        spawn.GuildId = guildId;
        spawn.AssetId = assetId;
        spawn.SpawnGuid = spawnId;
        spawn.SpawnType = GH_SPAWN_CREATURE;
        spawn.PhaseMask = phaseMask;

        spawn.MapId = mapId;
        spawn.X = x;
        spawn.Y = y;
        spawn.Z = z;
        spawn.O = o;

        spawn.w = w;

        house->Spawns.push_back(std::move(spawn));
    }
    
    return true;
}

// =====================================================
// GameObject
// =====================================================
bool GuildEnclaveSpawner::SpawnGameObject(uint32_t guildId, uint32_t assetId, uint32_t phaseMask, uint32_t mapId, uint32_t entry, float x, float y, float z, float o)
{
    Map* map = sMapMgr->CreateBaseMap(mapId);
    if (!map)
        return false;

    const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(entry);
    if (!objectInfo)
        return false;

    if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
        return false;

    float halfAngle = o * 0.5f;
    G3D::Quat rotation(0.0f, 0.0f, std::sin(halfAngle), std::cos(halfAngle));
    GameObject* object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
    //
    if (!object->Create(map->GenerateLowGuid<HighGuid::GameObject>(), objectInfo->entry, map, phaseMask, x, y, z, o, rotation, 255, GO_STATE_READY))
    {
        delete object;
        return false;
    }

    object->SaveToDB(mapId, (1 << map->GetSpawnMode()), phaseMask);
    uint32 spawnId = object->GetSpawnId();
    object->CleanupsBeforeDelete();
    delete object;

    object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
    if (!object->LoadGameObjectFromDB(spawnId, map, true))
    {
        delete object;
        return false;
    }

    sObjectMgr->AddGameobjectToGrid(spawnId, sObjectMgr->GetGameObjectData(spawnId));

    CharacterDatabase.Execute("INSERT INTO guildenclave_spawn (guildId,assetId,spawnGuid,spawnType,mapId,phaseMask,x,y,z,o) VALUES ({},{},{},{},{},{},{},{},{},{})",
        guildId, assetId, spawnId, GH_SPAWN_GAMEOBJECT, mapId, phaseMask, x, y, z, o);

    if (GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId))
    {
        GHGuildSpawn spawn;

        spawn.GuildId   = guildId;
        spawn.AssetId   = assetId;
        spawn.SpawnGuid = spawnId;
        spawn.SpawnType = GH_SPAWN_GAMEOBJECT;
        spawn.PhaseMask = phaseMask;

        spawn.MapId = mapId;
        spawn.X = x;
        spawn.Y = y;
        spawn.Z = z;
        spawn.O = o;

        spawn.w = 0;

        house->Spawns.push_back(std::move(spawn));
    }

    return true;
}

// =====================================================
// Remove asset
// =====================================================
bool GuildEnclaveSpawner::RemoveAsset(uint32_t guildId, uint32_t assetId)
{
    GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
    if (!house)
        return false;

    bool removed = false;
    for (auto itr = house->Spawns.begin(); itr != house->Spawns.end(); )
    {
        if (itr->AssetId != assetId)
        {
            ++itr;
            continue;
        }

        uint32_t spawnGuid = itr->SpawnGuid;
        bool success = false;

        switch (itr->SpawnType)
        {
            case GH_SPAWN_CREATURE:
                success = RemoveCreatureSpawn(spawnGuid);
                break;
            case GH_SPAWN_GAMEOBJECT:
                success = RemoveGameObjectSpawn(spawnGuid);
                break;
            default:
                break;
        }

        if (success)
        {
            CharacterDatabase.Execute("DELETE FROM guildenclave_spawn WHERE guildId={} AND assetId={} AND spawnGuid={}", guildId, assetId, spawnGuid);
            itr = house->Spawns.erase(itr);
            removed = true;
        }
        else
        {
            ++itr;
        }
    }

    return removed;
}

// =====================================================
// Remove all
// =====================================================
bool GuildEnclaveSpawner::RemoveAllAssets(uint32_t guildId)
{
    GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
    if (!house)
        return false;

    std::unordered_set<uint32_t> assetIds;

    for (GHGuildSpawn const& spawn : house->Spawns)
        assetIds.insert(spawn.AssetId);

    for (uint32_t assetId : assetIds)
        RemoveAsset(guildId, assetId);

    return true;
}

// =====================================================
// World removal
// =====================================================
bool GuildEnclaveSpawner::RemoveCreatureSpawn(uint32_t guid)
{
    CreatureData const* data = sObjectMgr->GetCreatureData(guid);
    if (!data)
        return false;

    if (Map* map = sMapMgr->FindBaseMap(data->mapid))
    {
        auto const& store = map->GetCreatureBySpawnIdStore();

        auto itr = store.find(guid);
        if (itr != store.end())
        {
            Creature* creature = itr->second;
            if (creature)
            {
                creature->CombatStop();
                creature->DeleteFromDB();
                creature->AddObjectToRemoveList();

                return true;
            }
        }
    }

    return false;
}

bool GuildEnclaveSpawner::RemoveGameObjectSpawn(uint32_t guid)
{
    GameObjectData const* data = sObjectMgr->GetGameObjectData(guid);
    if (!data)
        return false;

    if (Map* map = sMapMgr->FindBaseMap(data->mapid))
    {
        auto const& store = map->GetGameObjectBySpawnIdStore();

        auto itr = store.find(guid);
        if (itr != store.end())
        {
            GameObject* gameObject = itr->second;
            if (gameObject)
            {
                gameObject->DeleteFromDB();
                gameObject->AddObjectToRemoveList();

                return true;
            }
        }
    }

    return false;
}

