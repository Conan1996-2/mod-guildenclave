#include "Player.h"
#include "PlayerScript.h"
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

        if (!GuildEnclaveUtil::IsInGuildEnclaveArea(player))
            return;

        if(!EnterPhase(player))
            player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
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
    
        timer = 3000; // check once per second
        
        if (sGuildEnclaveMgr.IsMember(player))
        {
            if (player->GetGuildId() == 0)
            {
//                sGuildEnclaveMgr.LeavePhase (player);
                player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
            }
            else
            {
                sGuildEnclaveMgr.CheckBoundary(player);
                player->SetRestFlag(REST_FLAG_IN_CITY);
            }
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

        if (player->GetMapId() == location->MapId)
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
