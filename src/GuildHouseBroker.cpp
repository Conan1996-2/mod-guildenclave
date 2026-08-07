#include "GuildHouseBroker.h"

#include "GuildHouseNPCMovement.h"
#include "GuildHouseMgr.h"
#include "GuildHouseConfig.h"
#include "GuildHouseDefines.h"

#include "Guild.h"
#include "GossipDef.h"
#include "ScriptedGossip.h"
#include "Chat.h"

namespace
{

    enum GuildHouseActions
    {
        ACTION_NONE = 0,
        ACTION_TELEPORT = 1,
        ACTION_SELL = 2,
        ACTION_BUY_START = 1000,
        ACTION_CATEGORY_START = 2000,
        ACTION_CATALOG_START = 3000
    };

}

bool GuildHouseBroker::OnGossipHello(Player* player, Creature* creature)
{
    creature->GetMotionMaster()->MoveIdle();
    creature->m_Events.AddEvent(new ResumeCreatureMovementEvent(creature), creature->m_Events.CalculateTime(30000));
    
    ClearGossipMenuFor(player);

    Guild* guild = player->GetGuild();
    if (!guild)
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "You must be in a guild.", GOSSIP_SENDER_MAIN, ACTION_NONE);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    uint32 guildId = guild->GetId();

    if (!sGuildHouseMgr.HasGuildHouse(guildId))
    {
        if (GuildHouseUtil::IsGuildRank(player))
        {
            auto locations = sGuildHouseMgr.GetLocations();

            if (locations.empty())
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "No Guild House locations are available.", GOSSIP_SENDER_MAIN, ACTION_NONE);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Purchase a Guild House:", GOSSIP_SENDER_MAIN, ACTION_NONE);
                for (const GHLocation* location : locations)
                {
                    uint64 price = location->Price;
                    uint64 gold = price / GOLD;
                    uint64 silver = (price % GOLD) / SILVER;
                    uint64 copper = price % SILVER;
                    
                    std::string priceText = location->Name + " - ";
                    if (gold)
                        priceText += std::to_string(gold) + "G ";            
                    if (silver)
                        priceText += std::to_string(silver) + "S ";                
                    if (copper)
                        priceText += std::to_string(copper) + "C";
                    if (!gold && !silver && !copper)
                        priceText += "Free";
                    
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, priceText, GOSSIP_SENDER_MAIN, ACTION_BUY_START + location->Id);
                }
            }
        }
        else
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Your Guild Master must purchase the Guild House.", GOSSIP_SENDER_MAIN, ACTION_NONE);
        }
    }
    else
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Teleport to Guild House", GOSSIP_SENDER_MAIN, ACTION_TELEPORT);

        if (GuildHouseUtil::IsGuildRank(player))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Sell Guild House", GOSSIP_SENDER_MAIN, ACTION_SELL);
    }

    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    return true;
}

bool GuildHouseBroker::OnGossipSelect(Player* player, Creature* /*creature*/, uint32, uint32 action)
{
    ClearGossipMenuFor(player);

    Guild* guild = player->GetGuild();
    if (!guild)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Must be in a Guild.");
        return true;
    }

    uint32 guildId = guild->GetId();

    if (action >= ACTION_BUY_START && action < ACTION_CATEGORY_START)
    {
        uint32 locationId = action - ACTION_BUY_START;
        const GHLocation* location = sGuildHouseMgr.GetLocation(locationId);
        if (!location)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Invalid Guild House location.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (sGuildHouseMgr.HasGuildHouse(guildId))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Your guild already owns a Guild House.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (!GuildHouseUtil::IsGuildRank(player))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Only the Guild Master may purchase a Guild House.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (!sGuildHouseMgr.HasEnoughMoneyInGuild(guildId, location->Price)) // player->HasEnoughMoney(uint(location->Price)))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Your guild does not have enough gold.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (!sGuildHouseMgr.CreateGuildHouse(player, guildId, player->GetGUID().GetCounter(), locationId))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Failed to create Guild House.");
            CloseGossipMenuFor(player);
            return true;
        }       

        ChatHandler(player->GetSession()).PSendSysMessage("Guild House purchased: {}", location->Name);
        CloseGossipMenuFor(player);
        return true;
    }

    switch(action)
    {
        case ACTION_TELEPORT:
        {
            if (!sGuildHouseMgr.TeleportToGuildHouse(player))
                ChatHandler(player->GetSession()).PSendSysMessage("Unable to teleport to Guild House.");
            break;
        }

        case ACTION_SELL:
        {
            if (!GuildHouseUtil::IsGuildRank(player))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Only the Guild Master may sell the Guild House.");
                break;
            }

            const GHGuildHouse* house = sGuildHouseMgr.GetGuildHouse(guildId);
            if (!house)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Your guild does not own a Guild House.");
                break;
            }

            const GHLocation* location = sGuildHouseMgr.GetLocation(house->LocationId);
            if (!location)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Guild House location is invalid.");
                break;
            }

            if (!sGuildHouseMgr.SellGuildHouse(guildId))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Failed to sell Guild House.");
                break;
            }

            break;
        }

        case ACTION_NONE:
            return false;

        default:
            break;
    }
    
    CloseGossipMenuFor(player);
    return true;
}

void AddSC_GuildHouseBroker()
{
    new GuildHouseBroker();
}
