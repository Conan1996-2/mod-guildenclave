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

    static ChatCommandTable houseTable =
    {
        { "sell",       HandleSellGuildEnclave,     SEC_PLAYER, Console::No },
        { "tele",       HandleTeleportGuildEnclave, SEC_PLAYER, Console::No },
        { "teleport",   HandleTeleportGuildEnclave, SEC_PLAYER, Console::No }
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
        { "load",        HandleLoadBuild,        SEC_GAMEMASTER, Console::No },
        { "save",        HandleSaveBuild,        SEC_GAMEMASTER, Console::No },
        { "clear",       HandleClearBuild,       SEC_GAMEMASTER, Console::No },
        { "add",         HandleAddBuildAsset,    SEC_GAMEMASTER, Console::No },
        { "remove",      HandleRemoveBuildAsset, SEC_GAMEMASTER, Console::No }
    };

    static ChatCommandTable guildEnclaveTable =
    {
        { "npc",   npcTable },
        { "house", houseTable },
        { "asset", assetTable },
        { "shop",  shopTable },
        { "build", buildTable }
    };

    static ChatCommandTable root =
    {
        { "ge",          guildEnclaveTable },
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

    for (auto const& [localAssetId, databaseAssetId] : house->AssetIdMap)
    {
        auto assetItr = house->Assets.find(databaseAssetId);
        if (assetItr == house->Assets.end())
            continue;

        GHGuildAsset const& asset = assetItr->second;
        const GHCatalog* catalog = sGuildEnclaveCatalogMgr.GetCatalog(asset.CatalogId, player->GetTeamId());
        char const* statusText = "Unknown";

        switch (asset.Status)
        {
            case GH_ASSET_PURCHASED: statusText = "Purchased"; break;
            case GH_ASSET_PLACED: statusText = "Placed"; break;
            case GH_ASSET_STORED: statusText = "Stored"; break;
            case GH_ASSET_DISABLED: statusText = "Disabled"; break;
        }

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
    if (assetId < 100)
    {
        handler->PSendSysMessage("Invalid asset id.");
        return true;
    }

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
// Script Registration
// =====================================================
void AddSC_GuildEnclaveCommands()
{
    new GuildEnclaveCommandScript();
}
