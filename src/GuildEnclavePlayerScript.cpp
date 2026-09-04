#include "Player.h"
#include "PlayerScript.h"

#include "GuildEnclaveDefines.h"
#include "GuildEnclaveUtil.h"
#include "GuildEnclaveMgr.h"

class GuildEnclavePlayerScript : public PlayerScript
{
public:
    GuildEnclavePlayerScript() : PlayerScript("GuildEnclavePlayerScript") { }

    void OnPlayerLogin(Player* player) override
    {
        if (sGuildEnclaveMgr.IsMember(player))
            return;

        if (GuildEnclaveUtil::GetPlayerLocationId(player))
            if(!sGuildEnclaveMgr.EnterPhase(player))
                GuildEnclaveUtil::useStoneTeleport(player);
    }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        static std::unordered_map<ObjectGuid, uint32> timers;

        auto& timer = timers[player->GetGUID()];
        if (timer > diff)
        {
            timer -= diff;
            return;
        }
    
        timer = 3000; // check once per three seconds
        
        if (sGuildEnclaveMgr.IsMember(player) && GuildEnclaveUtil::IsInGuildEnclavePhase(player))
        {
            float dist = sGuildEnclaveMgr.OutsideBoundaryDistance(player);
            if (dist > 100)
                sGuildEnclaveMgr.LeavePhase(player);
            else
            {
                sGuildEnclaveMgr.CheckBoundary(player);
                player->SetRestFlag(REST_FLAG_IN_CITY);
            }
        } 
        else
        {
            uint32_t locationId = GuildEnclaveUtil::GetPlayerLocationId(player);
            if (locationId && !GuildEnclaveUtil::CanLocationPhase(locationId))
                if(!sGuildEnclaveMgr.EnterPhase(player))
                    GuildEnclaveUtil::useStoneTeleport(player);
        }
    }

    void OnPlayerMapChanged(Player* player) override
    {
        if (!sGuildEnclaveMgr.IsMember(player))
            return;

        const GHGuildEnclave* house = sGuildEnclaveMgr.GetGuildEnclave(player->GetGuildId());
        if (!house)
            return;

        GHLocation* location = sGuildEnclaveMgr.GetLocation(house->LocationId);
        if (!location)
            return;

        sGuildEnclaveMgr.LeavePhase(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        if (sGuildEnclaveMgr.IsMember(player))
            sGuildEnclaveMgr.LeavePhase(player);
    }
};

void AddSC_GuildEnclaveScripts()
{
    new GuildEnclavePlayerScript();
}
