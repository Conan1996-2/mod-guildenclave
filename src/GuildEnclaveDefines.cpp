#include "GuildEnclaveDefines.h"

#include "GuildEnclaveMgr.h"
#include "GuildEnclaveTypes.h"

#include "Player.h"
#include "Guild.h"

namespace GuildEnclaveUtil
{
/*
    GR_GUILDMASTER  = 0,
    GR_OFFICER      = 1,
    GR_VETERAN      = 2,
    GR_MEMBER       = 3,
    GR_INITIATE     = 4
*/

    // =====================================================
    // Does the Player have a guildenclave?
    // If not, return if its a guild master,
    // else return if it matches the the required guild rank
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
    // Is Player Inside Guild House
    //
    // Phase based validation.
    //
    // Checks:
    // - Guild membership
    // - Guild house ownership
    // - Correct map
    // - Location boundary
    // - Phase mask
    // =====================================================
    bool IsInGuildEnclave(Player* player)
    {
        if (!player)
            return false;
    
        Guild* guild = player->GetGuild();
        if (!guild)
            return false;
    
        uint32_t guildId = guild->GetId();
    
        const GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(guildId);
        if (!house)
            return false;
    
        const GHLocation* location = sGuildEnclaveMgr.GetLocation(house->LocationId);
        if (!location)
            return false;
    
        //
        // Must be in the correct map
        //
        if (player->GetMapId() != location->MapId)
            return false;
    
        //
        // Must be inside purchased location boundary
        //
        float x = player->GetPositionX();
        float y = player->GetPositionY();
    
        if (x < location->MinX || x > location->MaxX || y < location->MinY || y > location->MaxY)
        {
            return false;
        }
    
        //
        // Must have guild phase active
        //
        uint32_t phaseMask = sGuildEnclaveMgr.GetPhaseMask(guildId);
        if (!phaseMask)
            return false;
    
        return (player->GetPhaseMask() & phaseMask) != 0;
    }

}
