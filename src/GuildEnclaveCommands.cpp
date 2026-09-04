#include "GuildEnclaveCommands.h"

#include "GuildEnclaveDefines.h"
#include "GuildEnclaveTypes.h"
#include "GuildEnclaveUtil.h"
#include "GuildEnclaveMgr.h"
#include "GuildEnclaveCatalogMgr.h"
#include "GuildEnclaveSpawner.h"

#include "Chat.h"
#include "Player.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "Map.h"
#include "Guild.h"

#include <cstdlib>
#include <sstream>

GuildEnclaveCommandScript::GuildEnclaveCommandScript() : CommandScript("GuildEnclaveCommandScript")
{
}

// =====================================================
// Command Registration
// =====================================================
ChatCommandTable GuildEnclaveCommandScript::GetCommands() const
{
    static ChatCommandTable npcTable =
    {
        { "broker",     HandleAddBroker,   SEC_GAMEMASTER, Console::No },
        { "wander",     HandleWanderAsset, SEC_PLAYER, Console::No }
    };
    
    static ChatCommandTable boundaryTable =
    {
        { "north",      HandleEnclaveNorthBoundary,    SEC_GAMEMASTER, Console::No },
        { "south",      HandleEnclaveSouthBoundary,    SEC_GAMEMASTER, Console::No },
        { "east",       HandleEnclaveEastBoundary,     SEC_GAMEMASTER, Console::No },
        { "west",       HandleEnclaveWestBoundary,     SEC_GAMEMASTER, Console::No },
        { "northeast",  HandleEnclaveNEBoundary,       SEC_GAMEMASTER, Console::No },
        { "northwest",  HandleEnclaveNWBoundary,       SEC_GAMEMASTER, Console::No },
        { "southeast",  HandleEnclaveSEBoundary,       SEC_GAMEMASTER, Console::No },
        { "southwest",  HandleEnclaveSWBoundary,       SEC_GAMEMASTER, Console::No },
    };

    static ChatCommandTable setEnclaveTable =
    {
        { "portposition",   HandleEnclavePortPosition,  SEC_GAMEMASTER, Console::No },
        { "boundary",       HandleEnclaveBoundaries,    SEC_GAMEMASTER, Console::No },
        { "boundary",       boundaryTable },
        { "price",          HandleEnclavePrice,         SEC_GAMEMASTER, Console::No },
        { "enable",         HandleEnclaveEnable,        SEC_GAMEMASTER, Console::No },
        { "disable",        HandleEnclaveDisable,       SEC_GAMEMASTER, Console::No }
    };

    static ChatCommandTable enclaveTable =
    {
        { "sell",           HandleSellGuildEnclave,     SEC_PLAYER, Console::No },
        { "tele",           HandleTeleportGuildEnclave, SEC_PLAYER, Console::No },
        { "teleport",       HandleTeleportGuildEnclave, SEC_PLAYER, Console::No },
        { "list",           HandleListEnclaves,         SEC_GAMEMASTER, Console::No },
        { "create",         HandleNewEnclave,           SEC_GAMEMASTER, Console::No },
        { "set",            setEnclaveTable }
    };

    static ChatCommandTable assetTable =
    {
        { "list",       HandleListAssets,  SEC_PLAYER, Console::No },
        { "place",      HandlePlaceAsset,  SEC_PLAYER, Console::No },
        { "move",       HandleMoveAsset,   SEC_PLAYER, Console::No },
        { "store",      HandleStoreAsset,  SEC_PLAYER, Console::No },
        { "sell",       HandleSellAsset,   SEC_PLAYER, Console::No }
    };

    static ChatCommandTable shopTable =
    {
        { "categories", HandleListCategories,  SEC_PLAYER, Console::No },
        { "list",       HandleListCatalog,     SEC_PLAYER, Console::No },
        { "buy",        HandlePurchaseCatalog, SEC_PLAYER, Console::No }
    };

    static ChatCommandTable buildTable =
    {
        { "load",       HandleLoadBuild,        SEC_GAMEMASTER, Console::No },
        { "save",       HandleSaveBuild,        SEC_GAMEMASTER, Console::No },
        { "clear",      HandleClearBuild,       SEC_GAMEMASTER, Console::No },
        { "add",        HandleAddBuildAsset,    SEC_GAMEMASTER, Console::No },
        { "remove",     HandleRemoveBuildAsset, SEC_GAMEMASTER, Console::No }
    };

    static ChatCommandTable guildEnclaveTable =
    {
        { "npc",     npcTable },
        { "asset",   assetTable },
        { "shop",    shopTable },
        { "build",   buildTable },
        { "enclave", enclaveTable }
    };

    static ChatCommandTable root =
    {
        { "ge",            guildEnclaveTable },
        { "guildenclave",  guildEnclaveTable }
    };

    return root;
}

// =====================================================
// BROKER
// =====================================================
bool GuildEnclaveCommandScript::HandleAddBroker(ChatHandler* handler)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    uint32 entry = player->GetTeamId() == TEAM_ALLIANCE ? 900000 : 900001;
    if(sGuildEnclaveSpawner.SpawnCreature (0, 0,  player->GetPhaseMaskForSpawn(), player->GetMapId(), entry, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), 0))
        handler->PSendSysMessage("Guild House Broker permanently spawned.");
    else
        handler->PSendSysMessage("Unable to spawn Guild House Broker");

    return true;
}

// =====================================================
// Sell Guild House
// =====================================================
bool GuildEnclaveCommandScript::HandleSellGuildEnclave(ChatHandler* handler, char const*)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    uint32 guildId = player->GetGuildId();
    if (!guildId)
    {
        handler->PSendSysMessage("You must belong to a guild.");
        return true;
    }

    if (!GuildEnclaveUtil::IsGuildRank(player))
    {
        handler->PSendSysMessage("Only the Guild Master may sell the Guild House.");
        return true;
    }

    if (!sGuildEnclaveMgr.SellGuildEnclave(guildId))
    {
        handler->PSendSysMessage("Failed selling Guild House.");
        return true;
    }

    handler->PSendSysMessage("Guild House sold.");

    return true;
}

// =====================================================
// Teleport
// =====================================================
bool GuildEnclaveCommandScript::HandleTeleportGuildEnclave(ChatHandler* handler, char const*)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (!sGuildEnclaveMgr.TeleportToGuildEnclave(player))
    {
        handler->PSendSysMessage("Unable to teleport to Guild House.");
        return true;
    }

    return true;
}

// =====================================================
// List Assets
// =====================================================
bool GuildEnclaveCommandScript::HandleListAssets(ChatHandler* handler, char const*)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;
    
    uint32 guildId = player->GetGuildId();
    if (!guildId) 
    { 
        handler->PSendSysMessage("You must belong to a guild.");
        return true;
    }
    
    const GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
    if (!house)
    {
        handler->PSendSysMessage("Your guild does not own a Guild House.");
        return true;
    }
    
    handler->PSendSysMessage("==== Guild House Assets ====");
    if (house->Assets.empty())
    {
        handler->PSendSysMessage("No guild assets."); 
        return true;
    }
    
//    std::vector<uint32_t> assetIds;
//    assetIds.reserve(house->Assets.size());
//    for (auto const& [assetId, asset] : house->Assets)
//        assetIds.push_back(assetId);
    
//    std::sort(assetIds.begin(), assetIds.end());
    
//    for (uint32_t assetId : assetIds)
//    for (auto const& [localAssetId, databaseAssetId] : house->AssetIdMap)
//        {
//            auto assetItr = house->Assets.find(databaseAssetId);
//            if (assetItr == house->Assets.end())
//                continue;
        std::vector<uint32_t> localAssetIds;
        localAssetIds.reserve(house->AssetIdMap.size());
        
        for (auto const& [localAssetId, databaseAssetId] : house->AssetIdMap)
            localAssetIds.push_back(localAssetId);
        
        std::sort(localAssetIds.begin(), localAssetIds.end());
        
        for (uint32_t localAssetId : localAssetIds)
        {
            uint32_t databaseAssetId = house->AssetIdMap.at(localAssetId);
        
            auto assetItr = house->Assets.find(databaseAssetId);
            if (assetItr == house->Assets.end())
                continue;
            
            GHGuildAsset const& asset = assetItr->second;
            
//            GHGuildAsset const& asset = house->Assets.at(assetId);
            const GHCatalog* catalog = sGuildEnclaveCatalogMgr.GetCatalog(asset.CatalogId, player->GetTeamId());
            char const* statusText = "Unknown";
            
            switch (asset.Status)
                {
                    case GH_ASSET_PURCHASED: statusText = "Purchased"; break;
                    case GH_ASSET_PLACED: statusText = "Placed"; break; 
                    case GH_ASSET_STORED: statusText = "Stored"; break;
                    case GH_ASSET_DISABLED: statusText = "Disabled"; break;
                }
            
//            handler->PSendSysMessage("Asset {} | {} | {}", assetId, catalog ? catalog->Name.c_str() : "Unknown", statusText);
            handler->PSendSysMessage("Asset {} | {} | {}", localAssetId, catalog ? catalog->Name.c_str() : "Unknown", statusText);
        }
    return true;
}

// =====================================================
// Place Asset
// =====================================================
bool GuildEnclaveCommandScript::HandlePlaceAsset(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (!GuildEnclaveUtil::IsInGuildEnclave(player))
    {
        handler->PSendSysMessage("Must be in the purchased Guild Enclave to use.");
        return true;
    }
    
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .gh asset place <assetId>");
        return true;
    }

    uint32 assetId = atoi(args);
    if (!assetId)
    {
        handler->PSendSysMessage("Invalid asset id.");
        return true;
    }

    if (!sGuildEnclaveMgr.PlaceAsset(player, assetId, false))
    {
        handler->PSendSysMessage("Failed placing Guild House asset.");
        return true;
    }

    handler->PSendSysMessage("Guild House asset {} placed.", assetId);

    return true;
}

// =====================================================
// Move Asset
// =====================================================
bool GuildEnclaveCommandScript::HandleMoveAsset(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (!GuildEnclaveUtil::IsInGuildEnclave(player))
    {
        handler->PSendSysMessage("Must be in the purchased Guild Enclave to use.");
        return true;
    }
    
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .gh asset move <assetId>");
        return true;
    }

    uint32 assetId = atoi(args);
    if (!assetId)
    {
        handler->PSendSysMessage("Invalid asset id.");
        return true;
    }

    if (!sGuildEnclaveMgr.MoveAsset(player, assetId))
    {
        handler->PSendSysMessage("Failed moving Guild House asset.");
        return true;
    }

    handler->PSendSysMessage("Guild House asset {} moved.", assetId);

    return true;
}

// =====================================================
// Store Asset
// =====================================================
bool GuildEnclaveCommandScript::HandleStoreAsset(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .gh asset store <assetId>");
        return true;
    }

    uint32 assetId = atoi(args);
    if (!assetId)
    {
        handler->PSendSysMessage("Invalid asset id.");
        return true;
    }

    if (!sGuildEnclaveMgr.StoreAsset(player, assetId))
    {
        handler->PSendSysMessage("Failed storing Guild House asset.");
        return true;
    }

    handler->PSendSysMessage("Guild House asset {} stored.", assetId);

    return true;
}

// =====================================================
// Sell Asset
// =====================================================
bool GuildEnclaveCommandScript::HandleSellAsset(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .gh asset sell <assetId>");
        return true;
    }

    uint32 assetId = atoi(args);
    if (!sGuildEnclaveMgr.SellAsset(player, assetId))
    {
        handler->PSendSysMessage("Failed selling Guild House asset.");
        return true;
    }

    handler->PSendSysMessage("Guild House asset {} sold.", assetId);

    return true;
}

// =====================================================
// BUILD MANAGEMENT
// =====================================================
bool GuildEnclaveCommandScript::HandleLoadBuild(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (args && *args)
    {
        handler->PSendSysMessage("Usage: .gh build load");
        return true;
    }
    
    if (!sGuildEnclaveMgr.LoadBuild(player))
    {
        handler->PSendSysMessage("Failed loading all assets from database.");
        return true;
    }

    handler->PSendSysMessage("All previous assets removed and Default assets loaded for area build.");
    return true;
}

bool GuildEnclaveCommandScript::HandleSaveBuild(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (args && *args)
    {
        handler->PSendSysMessage("Usage: .gh build save");
        return true;
    }

    if (!sGuildEnclaveMgr.SaveBuild(player))
    {
        handler->PSendSysMessage("Failed saving all assets to database.");
        return true;
    }

    handler->PSendSysMessage("All assets have been saved to database for area build.");
    return true;
}

bool GuildEnclaveCommandScript::HandleClearBuild(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (args && *args)
    {
        handler->PSendSysMessage("Usage: .gh build clear");
        return true;
    }

    if (!sGuildEnclaveMgr.ClearBuild(player))
    {
        handler->PSendSysMessage("Failed removing all assets from database for area build.");
        return true;
    }

    handler->PSendSysMessage("All loaded assets have been erased.");
    return true;
}

bool GuildEnclaveCommandScript::HandleAddBuildAsset(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (!GuildEnclaveUtil::IsInGuildEnclave(player))
    {
        handler->PSendSysMessage("Must be in the purchased Guild Enclave to use.");
        return true;
    }

    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .gh build add <assetId>");
        return true;
    }

    uint32 assetId = atoi(args);
    if (!assetId)
    {
        handler->PSendSysMessage("Invalid asset id.");
        return true;
    }

    if (!sGuildEnclaveMgr.AddToBuild(player, assetId))
    {
        handler->PSendSysMessage("Failed adding asset to area build");
        return true;
    }

    handler->PSendSysMessage("Asset has been added area build.");
    return true;
}

bool GuildEnclaveCommandScript::HandleRemoveBuildAsset(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .gh build remove <assetId>");
        return true;
    }

    uint32 assetId = atoi(args);
    if (!assetId)
    {
        handler->PSendSysMessage("Invalid asset id.");
        return true;
    }
    
    if (!sGuildEnclaveMgr.RemoveFromBuild(player, assetId))
    {
        handler->PSendSysMessage("Failed removing asset from area build.");
        return true;
    }

    handler->PSendSysMessage("Asset has been removed from area build.");
    return true;
}

// =====================================================
// Wander Asset
//
// Usage:
// .ge asset wander <assetId> <distance>
//
// Sets the maximum wander distance for the asset.
// =====================================================
bool GuildEnclaveCommandScript::HandleWanderAsset(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge npc wander <assetId> <distance>");
        return true;
    }

    uint32 assetId = atoi(args);
    if (!assetId)
    {
        handler->PSendSysMessage("Invalid asset id.");
        return true;
    }

    char const* distanceArgs = strchr(args, ' ');
    if (!distanceArgs)
    {
        handler->PSendSysMessage("Usage: .ge npc wander <assetId> <distance>");
        return true;
    }

    while (*distanceArgs == ' ')
        ++distanceArgs;

    uint32 distance = atoi(distanceArgs);
    if (distance < 0)
    {
        handler->PSendSysMessage("Invalid wander distance.");
        return true;
    }

    if (!sGuildEnclaveMgr.WanderAsset(player, assetId, distance))
    {
        handler->PSendSysMessage("Failed setting wander distance for Guild House asset {}.", assetId);
        return true;
    }

    handler->PSendSysMessage("Guild House asset {} wander distance set to {} yards.", assetId, distance);
    return true;
}

// =====================================================
// List Root Categories
// =====================================================
bool GuildEnclaveCommandScript::HandleListCategories(ChatHandler* handler, char const*)
{
    std::vector<const GHCategory*> categories = sGuildEnclaveCatalogMgr.GetRootCategories();
    if (categories.empty())
    {
        handler->PSendSysMessage("No Guild House categories available.");
        return true;
    }

    handler->PSendSysMessage("==== Guild House Categories ====");

    for (const GHCategory* category : categories)
    {
        if (!category)
            continue;

        handler->PSendSysMessage("{} - {}", category->Id, category->Name.c_str());
    }

    return true;
}

// =====================================================
// List Catalog Items
//
// Usage:
// .gh shop list <categoryId>
// =====================================================
bool GuildEnclaveCommandScript::HandleListCatalog(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;
    
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .gh shop list <categoryId>");
        return true;
    }

    uint32 categoryId = atoi(args);
    if (!categoryId)
    {
        handler->PSendSysMessage("Invalid category id.");
        return true;
    }

    std::vector<const GHCatalog*> catalogs = sGuildEnclaveCatalogMgr.GetCatalogs(categoryId, player->GetTeamId());

    if (catalogs.empty())
    {
        handler->PSendSysMessage("No Guild House items found.");
        return true;
    }

    handler->PSendSysMessage("==== Guild House Catalog ====");

    for (const GHCatalog* catalog : catalogs)
    {
        if (!catalog)
            continue;

        handler->PSendSysMessage("{} - {}", catalog->CatalogId, catalog->Name.c_str());
    }

    return true;
}

// =====================================================
// Purchase Catalog Item
//
// Usage:
// .gh shop buy <catalogId>
//
// Purchase is handled by GuildEnclaveMgr.
// =====================================================
bool GuildEnclaveCommandScript::HandlePurchaseCatalog(ChatHandler* handler, char const* args)
{
    Player* player = handler->GetSession()->GetPlayer();
    if (!player)
        return false;

    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .gh shop buy <catalogId>");
        return true;
    }

    uint32 catalogId = atoi(args);
    if (!catalogId)
    {
        handler->PSendSysMessage("Invalid catalog id.");
        return true;
    }

    if (!sGuildEnclaveMgr.PurchaseCatalogItem(player, catalogId))
    {
        handler->PSendSysMessage("Failed purchasing Guild House item.");
        return true;
    }

    handler->PSendSysMessage("Guild House item purchased.");

    return true;
}

// =====================================================
// List All Guild Enclaves and if Active
// =====================================================
bool GuildEnclaveCommandScript::HandleListEnclaves(ChatHandler* handler, char const*)
{
    std::vector<const GHLocation*> locations = sGuildEnclaveMgr.GetLocations(true);

    handler->PSendSysMessage("==== Guild Enclaves ====");

    if (locations.empty())
    {
        handler->PSendSysMessage("No Guild Enclaves loaded.");
        return true;
    }

    std::sort(locations.begin(), locations.end(), [](const GHLocation* a, const GHLocation* b) { return a->Id < b->Id; });

    for (const GHLocation* location : locations)
    {
        if (!location)
            continue;

        handler->PSendSysMessage("LocationId: {} | Name: {} | Map: {} | {}", location->Id, location->Name.c_str(), location->MapId, location->Enabled ? "Enabled" : "Disabled");
    }

    return true;
}

// =====================================================
// Adds a new Guild Enclaves and sets it deactivated by default
// =====================================================
bool GuildEnclaveCommandScript::HandleNewEnclave(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave create \"<name>\"");
        return true;
    }

    std::string input(args);
    if (input.front() != '"')
    {
        handler->PSendSysMessage("Enclave name must be enclosed in double quotes.");
        return true;
    }

    size_t closingQuote = input.find('"', 1);
    if (closingQuote == std::string::npos)
    {
        handler->PSendSysMessage("Enclave name must be enclosed in double quotes.");
        return true;
    }

    if (!input.substr(closingQuote + 1).empty())
    {
        handler->PSendSysMessage("Usage: .ge enclave create \"<name>\"");
        return true;
    }

    std::string name = input.substr(1, closingQuote - 1);
    if (name.empty())
    {
        handler->PSendSysMessage("Enclave name cannot be empty.");
        return true;
    }

    uint32_t locationId = sGuildEnclaveMgr.CreateLocation(name, handler->GetSession()->GetPlayer());
    if (!locationId)
    {
        handler->PSendSysMessage("Failed creating Guild Enclave.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave and port {} created with ID: {}.", name.c_str(), locationId);

    return true;
}

// =====================================================
// Set the port position to the players current position
// =====================================================
bool GuildEnclaveCommandScript::HandleEnclavePortPosition(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave portposition <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    if(!sGuildEnclaveMgr.SetEnclavePortPosition(locationId, handler->GetSession()->GetPlayer()))
    {
        handler->PSendSysMessage("Failed setting GuildEnclave port position.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave {} port position set", locationId);

    return true;
}

bool GuildEnclaveCommandScript::HandleEnclaveNorthBoundary(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary north <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary north <locationId>");
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    if(!sGuildEnclaveMgr.SetEnclaveBorderBoundary(locationId, handler->GetSession()->GetPlayer(), GH_MAP_NORTH))
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary north <locationId>");
        handler->PSendSysMessage("Failed setting GuildEnclave border position.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave North position set");

    return true;
}

bool GuildEnclaveCommandScript::HandleEnclaveSouthBoundary(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary south <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary south <locationId>");
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    if(!sGuildEnclaveMgr.SetEnclaveBorderBoundary(locationId, handler->GetSession()->GetPlayer(), GH_MAP_SOUTH))
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary south <locationId>");
        handler->PSendSysMessage("Failed setting GuildEnclave border position.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave South position set");

    return true;
}

bool GuildEnclaveCommandScript::HandleEnclaveEastBoundary(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary east <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary east <locationId>");
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    if(!sGuildEnclaveMgr.SetEnclaveBorderBoundary(locationId, handler->GetSession()->GetPlayer(), GH_MAP_EAST))
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary east <locationId>");
        handler->PSendSysMessage("Failed setting GuildEnclave border position.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave East position set");

    return true;
}

bool GuildEnclaveCommandScript::HandleEnclaveWestBoundary(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary west <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary west <locationId>");
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    if(!sGuildEnclaveMgr.SetEnclaveBorderBoundary(locationId, handler->GetSession()->GetPlayer(), GH_MAP_WEST))
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary west <locationId>");
        handler->PSendSysMessage("Failed setting GuildEnclave border position.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave West position set");

    return true;
}

bool GuildEnclaveCommandScript::HandleEnclaveNEBoundary(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary northeast <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary northeast <locationId>");
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    if(!sGuildEnclaveMgr.SetEnclaveBorderBoundary(locationId, handler->GetSession()->GetPlayer(), GH_MAP_NORTH | GH_MAP_EAST))
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary northeast <locationId>");
        handler->PSendSysMessage("Failed setting GuildEnclave border position.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave northeast position set");

    return true;
}

bool GuildEnclaveCommandScript::HandleEnclaveNWBoundary(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary northwest <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary northwest <locationId>");
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    if(!sGuildEnclaveMgr.SetEnclaveBorderBoundary(locationId, handler->GetSession()->GetPlayer(), GH_MAP_NORTH | GH_MAP_WEST))
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary northwest <locationId>");
        handler->PSendSysMessage("Failed setting GuildEnclave border position.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave northwest position set");

    return true;
}

bool GuildEnclaveCommandScript::HandleEnclaveSEBoundary(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary southeast <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary southeast <locationId>");
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    if(!sGuildEnclaveMgr.SetEnclaveBorderBoundary(locationId, handler->GetSession()->GetPlayer(), GH_MAP_SOUTH | GH_MAP_EAST))
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary southeast <locationId>");
        handler->PSendSysMessage("Failed setting GuildEnclave border position.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave southeast position set");

    return true;
}

bool GuildEnclaveCommandScript::HandleEnclaveSWBoundary(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary southwest <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary southwest <locationId>");
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    if(!sGuildEnclaveMgr.SetEnclaveBorderBoundary(locationId, handler->GetSession()->GetPlayer(), GH_MAP_SOUTH | GH_MAP_WEST))
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundary southwest <locationId>");
        handler->PSendSysMessage("Failed setting GuildEnclave border position.");
        return true;
    }

    handler->PSendSysMessage("Guild Enclave southwest position set");

    return true;
}

// =====================================================
// Enables the ability to use and show a GuildEnclave for purchase
// =====================================================
bool GuildEnclaveCommandScript::HandleEnclaveBoundaries(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundaries <locationId> <minX> <minY> <maxX> <maxY>");
        return true;
    }

    std::stringstream ss(args);
    uint32_t locationId = 0;
    float minX = 0.0f;
    float maxX = 0.0f;
    float minY = 0.0f;
    float maxY = 0.0f;

    if (!(ss >> locationId >> minX >> minY >> maxX >> maxY))
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundaries <locationId> <minX> <minY> <maxX> <maxY>");
        return true;
    }
    
    std::string extra;
    if (ss >> extra)
    {
        handler->PSendSysMessage("Usage: .ge enclave set boundaries <locationId> <minX> <minY> <maxX> <maxY>");
        return true;
    }
    
    if (!locationId)
    {
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    GHLocation* location = sGuildEnclaveMgr.GetLocation(locationId);
    if (!location)
    {
        handler->PSendSysMessage("Guild Enclave location {} does not exist.", locationId);
        return true;
    }

    if (!sGuildEnclaveMgr.SetEnclaveBoundaries(locationId, minX, minY, maxX, maxY))
    {
        handler->PSendSysMessage("Failed to set boundaries for Guild Enclave {}.", locationId);
        return true;
    }

    handler->PSendSysMessage("Guild Enclave {} '{}' boundaries updated: MinX: {} MaxX: {} MinY: {} MaxY: {}",locationId, location->Name, minX, maxX, minY, maxY);

    return true;
}

// =====================================================
// Enables the ability to use and show a GuildEnclave for purchase
// =====================================================
bool GuildEnclaveCommandScript::HandleEnclavePrice(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave set setprice <locationId> <price>");
        return true;
    }

    std::stringstream ss(args);
    uint32_t locationId = 0;
    uint64_t amount = 0;
    
    if (!(ss >> locationId >> amount))
    {
        handler->PSendSysMessage("Usage: .ge enclave set setprice <locationId> <price>");
        return true;
    }

    std::string extra;
    if (ss >> extra)
    {
        handler->PSendSysMessage("Usage: .ge enclave set setprice <locationId> <price>");
        return true;
    }

    if (!sGuildEnclaveMgr.SetEnclavePrice(locationId, amount))
    {
        handler->PSendSysMessage("Failed to set price for Guild Enclave location {}.", locationId);
        return true;
    }

    handler->PSendSysMessage("Guild Enclave location {} price set to {}.", locationId, amount);

    return true;
}

// =====================================================
// Enables the ability to use and show a GuildEnclave for purchase
// =====================================================
bool GuildEnclaveCommandScript::HandleEnclaveEnable(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave enable <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    const GHLocation* location = sGuildEnclaveMgr.GetLocation(locationId);
    if (!location)
    {
        handler->PSendSysMessage("Guild Enclave location {} does not exist.", locationId);
        return true;
    }

    if (location->Enabled)
    {
        handler->PSendSysMessage("Guild Enclave {} is already enabled.", locationId);
        return true;
    }

    if (!sGuildEnclaveMgr.SetLocationEnabled(locationId, true))
    {
        handler->PSendSysMessage("Failed to enable Guild Enclave {}.", locationId);
        return true;
    }

    handler->PSendSysMessage("Guild Enclave {} '{}' has been enabled.", locationId, location->Name);

    return true;
}

// =====================================================
// Adds a new Guild Enclaves and sets it deactivated by default
// =====================================================
bool GuildEnclaveCommandScript::HandleEnclaveDisable(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        handler->PSendSysMessage("Usage: .ge enclave disable <locationId>");
        return true;
    }

    uint32_t locationId = std::atoi(args);
    if (!locationId)
    {
        handler->PSendSysMessage("Invalid location ID.");
        return true;
    }

    const GHLocation* location = sGuildEnclaveMgr.GetLocation(locationId);
    if (!location)
    {
        handler->PSendSysMessage("Guild Enclave location {} does not exist.", locationId);
        return true;
    }

    if (!location->Enabled)
    {
        handler->PSendSysMessage("Guild Enclave {} is already disabled.", locationId);
        return true;
    }

    std::string name = location->Name;
    if (!sGuildEnclaveMgr.SetLocationEnabled(locationId, false))
    {
        handler->PSendSysMessage("Failed to disable Guild Enclave {}.", locationId);
        return true;
    }

    handler->PSendSysMessage("Guild Enclave {} '{}' has been disabled.", locationId, name);

    return true;
}

// =====================================================
// Script Registration
// =====================================================
void AddSC_GuildEnclaveCommands()
{
    new GuildEnclaveCommandScript();
}
