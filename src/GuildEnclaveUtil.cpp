#include "GuildEnclaveUtil.h"
#include "GuildEnclaveMgr.h"
#include "GuildEnclaveTypes.h"

#include "Guild.h"
#include "Player.h"

namespace GuildEnclaveUtil
{
    
    // =====================================================
    // Is Player Inside Guild Enclave
    // =====================================================

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

    bool IsInGuildEnclave(Player* player)
    {
        if (!IsInGuildEnclaveArea(player))
            return false;

        return sGuildEnclaveMgr.IsMember(player);
    }
    
    // =====================================================
    // Guild Phase Validation
    // =====================================================
    
    bool IsGuildEnclavePhase(uint32_t guildId, uint32_t phaseMask)
    {
        uint32_t guildPhase = sGuildEnclaveMgr.GetPhaseMask(guildId);
        if (!guildPhase)
            return false;
    
        return (phaseMask & guildPhase) != 0;
    }
    
}
