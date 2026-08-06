#include "GuildHouseConfig.h"

#include "Config.h"
#include "Log.h"

GuildHouseConfig& GuildHouseConfig::Instance()
{
    static GuildHouseConfig instance;
    return instance;
}

void GuildHouseConfig::Load()
{
    _enabled = sConfigMgr->GetOption<bool>("GuildHouse.Enable", false);
    _free = sConfigMgr->GetOption<bool>("GuildHouse.Free", false);
    _allWander = sConfigMgr->GetOption<bool>("GuildHouse.NPC.AllWander", false);

    _refundPercent = sConfigMgr->GetOption<float>("GuildHouse.Refund", .8);

    if (_refundPercent > 1)
        _refundPercent = 1;

    LOG_INFO("server.loading", "GuildHouse: Configuration loaded.");
}

bool GuildHouseConfig::IsEnabled() const
{
    return _enabled;
}

bool GuildHouseConfig::IsFree() const
{
    return _free > 0;
}

bool GuildHouseConfig::LoadAll() const
{
    return _free == 2;
}

double GuildHouseConfig::GetRefundPercent() const
{
    return _refundPercent;
}

bool GuildHouseconfig::isWanderer() const
{
    return _allWander;
}
