#include "GuildEnclaveSalesman.h"

#include "GuildEnclaveDefines.h"
#include "GuildEnclaveUtil.h"
#include "GuildEnclaveMgr.h"
#include "GuildEnclaveCatalogMgr.h"
#include "GuildEnclaveNPCMovement.h"

#include "Player.h"
#include "Creature.h"
#include "Guild.h"

#include "GossipDef.h"
#include "ScriptedGossip.h"

#include "Chat.h"

// =====================================================
// Validate Access
//
// Salesman exists inside one guild bit phase.
//
// Rules:
// - Player must have guild
// - Guild must own house
// - Player must be inside guild phase
// - Creature must be inside same guild phase
// - Player must be inside boundary
//
// =====================================================
bool GuildEnclaveSalesman::ValidateSalesmanAccess(Player* player, Creature* creature)
{
    if (!player || !creature)
        return false;

    Guild* guild = player->GetGuild();
    if (!guild)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("You must belong to a guild.");
        return false;
    }

    uint32 guildId = guild->GetId();
    if (!sGuildEnclaveMgr.HasGuildEnclave(guildId))
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Your guild does not own a Guild House.");
        return false;
    }

    if (!GuildEnclaveUtil::IsGuildRank(player))
    {
        ChatHandler(player->GetSession()).PSendSysMessage("You do not have the authority to access the Salesman.");
        return false;
    }
    
    uint32 phaseMask = sGuildEnclaveMgr.GetPhaseMask(guildId);
    if (!phaseMask)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Guild House phase unavailable.");
        return false;
    }

    if (player->GetPhaseMask() != phaseMask)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("You must be inside your Guild House.");
        return false;
    }

    if (creature->GetPhaseMask() != phaseMask)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("This salesman belongs to another Guild House.");
        return false;
    }

    if (!GuildEnclaveUtil::IsInsideGuildEnclaveBoundary(player))
    {
        ChatHandler(player->GetSession()).PSendSysMessage("You are outside the Guild House area.");
        return false;
    }

    return true;
}

// =====================================================
// Confirmation of sale
// =====================================================
void GuildEnclaveSalesman::SendPurchaseConfirmMenu(Player* player, Creature* creature, uint32 catalogId)
{

    ChatHandler(player->GetSession()).PSendSysMessage("In Confirmation.");

    const GHCatalog* catalog = sGuildEnclaveCatalogMgr.GetCatalog(catalogId);
    if (!catalog)
    {
        CloseGossipMenuFor(player);
        return;
    }

    std::string cost = "Cost: " + GuildEnclaveUtil::GoldToString(catalog->Price);

    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Purchase " + catalog->Name + " - " + cost, GOSSIP_SENDER_MAIN, ACTION_CONFIRM + catalogId);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<< Back", GOSSIP_SENDER_MAIN, ACTION_BACK + catalog->CategoryId);
    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
}

// =====================================================
// Confirmation of Premade sale
// =====================================================
void GuildEnclaveSalesman::SendPreMadePurchaseConfirmMenu(Player* player, Creature* creature)
{
    ChatHandler(player->GetSession()).PSendSysMessage("In Confirmation.");

    const GHLocation* location = sGuildEnclaveMgr.GetGuildLocation (player->GetGuildId());
    if (!location)
    {
        CloseGossipMenuFor(player);
        return;
    }
    
    std::string cost = GuildEnclaveUtil::GoldToString(sGuildEnclaveMgr.GetTotalAssetCost(player, location->Id));

    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Purchase Total Assets - " + cost, GOSSIP_SENDER_MAIN, ACTION_CONFIRM);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<< Back", GOSSIP_SENDER_MAIN, ACTION_BACK);
    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
}

// =====================================================
// Gossip Hello
// =====================================================
bool GuildEnclaveSalesman::OnGossipHello(Player* player, Creature* creature)
{
    creature->GetMotionMaster()->MoveIdle();
    creature->m_Events.AddEvent(new ResumeCreatureMovementEvent(creature), creature->m_Events.CalculateTime(30000));
    
    ClearGossipMenuFor(player);

    if (!ValidateSalesmanAccess(player, creature))
    {
        CloseGossipMenuFor(player);
        return true;
    }

    SendCatalogMenu(player, creature);

    return true;
}

// =====================================================
// Root Catalog Menu
// =====================================================
void GuildEnclaveSalesman::SendCatalogMenu(Player* player, Creature* creature)
{
    auto categories = sGuildEnclaveCatalogMgr.GetRootCategories();

    for (const GHCategory* category : categories)
    {
        if (!category)
            continue;

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, category->Name, GOSSIP_SENDER_MAIN, ACTION_CATEGORY_START + category->Id);
    }

    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "---------------------------", GOSSIP_SENDER_MAIN, 0);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Default Guild Enclave", GOSSIP_SENDER_MAIN, ACTION_PREBUILT_START);

    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
}

// =====================================================
// Gossip Selection
// =====================================================
bool GuildEnclaveSalesman::OnGossipSelect(Player* player, Creature* creature, uint32, uint32 action)
{
    ClearGossipMenuFor(player);

    if (!ValidateSalesmanAccess(player, creature))
    {
        CloseGossipMenuFor(player);
        return true;
    }

    if (action >= ACTION_CONFIRM)
    {
        uint32 catalogId = action - ACTION_CONFIRM;
    
        if (!sGuildEnclaveMgr.PurchaseCatalogItem(player, catalogId))
            ChatHandler(player->GetSession()).PSendSysMessage("Unable to purchase item.");
    
        CloseGossipMenuFor(player);
        return true;
    }
    
    if (action >= ACTION_BACK)
    {
        uint32 parentId = action - ACTION_BACK;
    
        if (parentId == 0)
            SendCatalogMenu(player, creature);
        else
            SendCategoryMenu(player, creature, parentId);
    
        return true;
    }

    if (action >= ACTION_CATEGORY_START && action < ACTION_CATALOG_START)
    {
        uint32 categoryId = action - ACTION_CATEGORY_START;
        SendCategoryMenu(player, creature, categoryId);
        return true;
    }

    if (action >= ACTION_CATALOG_START && action < ACTION_BACK)
    {
        uint32 catalogId = action - ACTION_CATALOG_START;
    
        SendPurchaseConfirmMenu(player, creature, catalogId);
        return true;
    }

    if(action >= ACTION_PREBUILT_START)
    {
        SendPreMadePurchaseConfirmMenu (player, creature);
        return true;
    }
    
    CloseGossipMenuFor(player);

    return true;
}

// =====================================================
// Category Menu
// =====================================================
void GuildEnclaveSalesman::SendCategoryMenu(Player* player, Creature* creature, uint32 categoryId)
{
    auto children = sGuildEnclaveCatalogMgr.GetChildCategories( categoryId);
    const GHCategory* current = sGuildEnclaveCatalogMgr.GetCategory(categoryId);
    
    if (current)
    {
        if (current->ParentId != 0)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<< Back", GOSSIP_SENDER_MAIN, ACTION_BACK + current->ParentId);
        else
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<< Back", GOSSIP_SENDER_MAIN, ACTION_BACK);
    }
    
    for (const GHCategory* child : children)
    {
        if (!child)
            continue;

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, child->Name, GOSSIP_SENDER_MAIN, ACTION_CATEGORY_START + child->Id);
    }

    auto catalogs = sGuildEnclaveCatalogMgr.GetCatalogs(categoryId, player->GetTeamId());
    for (const GHCatalog* catalog : catalogs)
    {
        if (!catalog)
            continue;

        std::string displayName = catalog->Name + " - " + GuildEnclaveUtil::GoldToString(catalog->Price);
        
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, displayName, GOSSIP_SENDER_MAIN, ACTION_CATALOG_START + catalog->CatalogId);
    }

    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
}

// =====================================================
// Registration
// =====================================================
void AddSC_GuildEnclaveSalesman()
{
    new GuildEnclaveSalesman();
}
