#include "GuildEnclaveConfig.h"

#include "Config.h"
#include "Log.h"

GuildEnclaveConfig& GuildEnclaveConfig::Instance()
{
    static GuildEnclaveConfig instance;
    return instance;
}

void GuildEnclaveConfig::Load()
{
    _enabled = sConfigMgr->GetOption<bool>("GuildEnclave.Enable", false);
    _free = sConfigMgr->GetOption<bool>("GuildEnclave.Free", false);    
    _allWander = sConfigMgr->GetOption<bool>("GuildEnclave.NPCWander.Enabled", false);
    _wanderDistance = sConfigMgr->GetOption<int16_t>("GuildEnclave.NPCWander.Distance", 0);

    _refundPercent = sConfigMgr->GetOption<float>("GuildEnclave.Refund", .8);
    if (_refundPercent > 1)
        _refundPercent = 1;

    LOG_INFO("server.loading", "GuildEnclave: Configuration loaded.");
}

bool GuildEnclaveConfig::IsEnabled() const
{
    return _enabled;
}

bool GuildEnclaveConfig::IsFree() const
{
    return _free > 0;
}

bool GuildEnclaveConfig::LoadAll() const
{
    return _free == 2;
}

double GuildEnclaveConfig::GetRefundPercent() const
{
    return _refundPercent;
}

bool GuildEnclaveConfig::AllWander() const
{
    return _allWander;
}

int16_t GuildEnclaveConfig::WanderDistance() const
{
    return _wanderDistance;
}
