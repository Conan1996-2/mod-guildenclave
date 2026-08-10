#include "Player.h"
#include "PlayerScript.h"
#include "GuildHouseMgr.h"

class GuildHousePlayerScript : public PlayerScript
{
public:
    GuildHousePlayerScript() : PlayerScript("GuildHousePlayerScript") { }

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
        
        if (sGuildHouseMgr.IsMember(player))
        {
            sGuildHouseMgr.CheckBoundary(player);
            player->SetRestFlag(REST_FLAG_IN_CITY);
        }
    }

    void OnPlayerMapChanged(Player* player) override
    {
        if (!sGuildHouseMgr.IsMember(player))
            return;

        const GHGuildHouse* house = sGuildHouseMgr.GetGuildHouse(player->GetGuildId());
        if (!house)
            return;

        GHLocation* location = sGuildHouseMgr.GetLocation(house->LocationId);
        if (!location)
            return;

        if (player->GetMapId() == location->MapId)
            return;
        
        sGuildHouseMgr.LeavePhase(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        if (sGuildHouseMgr.IsMember(player))
            sGuildHouseMgr.LeavePhase(player);
    }
};

void AddSC_GuildHouseScripts()
{
    new GuildHousePlayerScript();
}
