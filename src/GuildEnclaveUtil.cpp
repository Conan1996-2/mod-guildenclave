#include "GuildEnclaveUtil.h"
#include "GuildEnclaveMgr.h"
#include "GuildEnclaveTypes.h"

#include "Guild.h"
#include "Player.h"

#include <string>

namespace GuildEnclaveUtil
{

    inline bool HasFlag(uint32_t value, uint32_t flag)
    {
        return (value & flag) != 0;
    }

    inline bool IsAlliance(uint32_t flags)
    {
        return HasFlag(flags, GH_FACTION_ALLIANCE);
    }

    inline bool IsHorde(uint32_t flags)
    {
        return HasFlag(flags, GH_FACTION_HORDE);
    }

    inline bool IsNeutral(uint32_t flags)
    {
        return HasFlag(flags, GH_FACTION_NEUTRAL);
    }

    // =====================================================
    // Is Player Inside Guild Enclave AREA, does not have to be a member
    // =====================================================
    bool IsInsideGuildEnclaveBoundary(Player* player)
    {
        const GHLocation* location = sGuildEnclaveMgr.GetGuildLocation(player->GetGuildId());
        if(!location)
            return false;
    
        float x = player->GetPositionX();
        float y = player->GetPositionY();    
        return (x >= location->MinX && x <= location->MaxX && y >= location->MinY && y <= location->MaxY)
    }

    bool IsInGuildEnclaveArea(Player* player)
    {
        if (!player)
            return false;
    
        Guild* guild = player->GetGuild();
        if (!guild)
            return false;
    
        uint32 guildId = guild->GetId();
    
        const GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
        if (!house)
            return false;
    
        const GHLocation* location = sGuildEnclaveMgr.GetLocation(house->LocationId);
        if (!location)
            return false;
    
        if (player->GetMapId() != location->MapId)
            return false;
    
        float x = player->GetPositionX();
        float y = player->GetPositionY();
        if (x < location->MinX || x > location->MaxX || y < location->MinY || y > location->MaxY)
            return false;
        
        return true;
    }

    // =====================================================
    // Guild Phase Validation
    // =====================================================
    bool IsInGuildEnclavePhase(Player* player)
    {
        if (!player)
            return false;
    
        Guild* guild = player->GetGuild();
        if (!guild)
            return false;
    
        uint32_t guildPhase = sGuildEnclaveMgr.GetPhaseMask(guild->GetId());
        if (!guildPhase)
            return false;
    
        return (player->GetPhaseMask() & guildPhase) != 0;
    }

    // =====================================================
    // Is Player Inside Guild Enclave AREA, PHASE and a member
    // =====================================================
    bool IsInGuildEnclave(Player* player)
    {
        if (!IsInGuildEnclaveArea(player))
            return false;

        if (!IsInGuildEnclavePhase(player))
            return false;
        
        return sGuildEnclaveMgr.IsMember(player);
    }


    // =====================================================
    // Check if player has a valid Guild Enclave
    // =====================================================
    bool HasGuildEnclave(Player* player)
    {
        if (!player)
            return false;
    
        uint32_t guildId = player->GetGuildId();
        if (!guildId)
            return false;
    
        GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
        if (!house)
            return false;
    
        const GHLocation* location = sGuildEnclaveMgr.GetGuildLocation(guildId);
        if (!location)
            return false;

        return true;
    }
    
    // =====================================================
    // Is the player proper guild rank
    // =====================================================
    bool IsGuildRank(Player* player)
    {
        if (!player)
            return false;
    
        Guild* guild = player->GetGuild();
        if (!guild)
            return false;

        Guild::Member* member = guild->GetMember(player->GetGUID());
        if (!member)
            return false;

        uint32_t guildId = guild->GetId();
        const GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
        if (!house)
            return guild->GetLeaderGUID() == player->GetGUID();
        
        return member->GetRankId() <= house->RequiredGuildRank;
    }

    // =====================================================
    // Is the player Allowed to place objects
    // =====================================================
    inline bool CanManageGuildEnclave(Player* player)
    {
        return IsGuildRank(player) && IsInGuildEnclave(player);
    }

    // =====================================================
    // Money Calculations
    // =====================================================
    std::string GoldToString (uint64_t price)
    {
        uint64_t gold = price / GOLD;
        uint64_t silver = (price % GOLD) / SILVER;
        uint64_t copper = price % SILVER;
        
        std::string priceText = "";
        if (gold)
            priceText += std::to_string(gold) + "G ";            
        if (silver)
            priceText += std::to_string(silver) + "S ";                
        if (copper)
            priceText += std::to_string(copper) + "C";
        if (!gold && !silver && !copper)
            priceText = "Free";

        return priceText;
    }
}
