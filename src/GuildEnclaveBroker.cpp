#include "GuildEnclaveBroker.h"

#include "GuildEnclaveConfig.h"
#include "GuildEnclaveDefines.h"
#include "GuildEnclaveUtil.h"
#include "GuildEnclaveNPCMovement.h"
#include "GuildEnclaveMgr.h"

#include "Guild.h"
#include "GossipDef.h"
#include "ScriptedGossip.h"
#include "Chat.h"

bool GuildEnclaveBroker::OnGossipHello(Player* player, Creature* creature)
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

    if (!sGuildEnclaveMgr.HasGuildEnclave(guildId))
    {
        if (GuildEnclaveUtil::IsGuildRank(player))
        {
            auto locations = sGuildEnclaveMgr.GetLocations(false);

            if (locations.empty())
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "No Guild House locations are available.", GOSSIP_SENDER_MAIN, ACTION_NONE);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Purchase a Guild House:", GOSSIP_SENDER_MAIN, ACTION_NONE);
                for (const GHLocation* location : locations)
                {
                    std::string priceText = location->Name + " - " + GuildEnclaveUtil::GoldToString(location->Price);
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

        if (GuildEnclaveUtil::IsGuildRank(player))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Sell Guild House", GOSSIP_SENDER_MAIN, ACTION_SELL);
    }

    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    return true;
}

bool GuildEnclaveBroker::OnGossipSelect(Player* player, Creature* /*creature*/, uint32, uint32 action)
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
        const GHLocation* location = sGuildEnclaveMgr.GetLocation(locationId);
        if (!location)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Invalid Guild House location.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (sGuildEnclaveMgr.HasGuildEnclave(guildId))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Your guild already owns a Guild House.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (!GuildEnclaveUtil::IsGuildRank(player))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Only the Guild Master may purchase a Guild House.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (!sGuildEnclaveMgr.HasEnoughMoneyInGuild(guildId, location->Price)) // player->HasEnoughMoney(uint(location->Price)))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Your guild does not have enough gold.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (!sGuildEnclaveMgr.CreateGuildEnclave(player, guildId, player->GetGUID().GetCounter(), locationId))
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
            if (!sGuildEnclaveMgr.TeleportToGuildEnclave(player))
                ChatHandler(player->GetSession()).PSendSysMessage("Unable to teleport to Guild House.");
            break;
        }

        case ACTION_SELL:
        {
            if (!GuildEnclaveUtil::IsGuildRank(player))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Only the Guild Master may sell the Guild House.");
                break;
            }

            const GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
            if (!house)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Your guild does not own a Guild House.");
                break;
            }

            const GHLocation* location = sGuildEnclaveMgr.GetLocation(house->LocationId);
            if (!location)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Guild House location is invalid.");
                break;
            }

            if (!sGuildEnclaveMgr.SellGuildEnclave(guildId))
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

void AddSC_GuildEnclaveBroker()
{
    new GuildEnclaveBroker();
}
