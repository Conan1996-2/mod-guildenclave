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
     LOG_INFO("server.loading", "");
    _enabled = sConfigMgr->GetOption<bool>("GuildEnclave.Enable", false);
    _free = sConfigMgr->GetOption<bool>("GuildEnclave.Free", false);    
    _allWander = sConfigMgr->GetOption<bool>("GuildEnclave.NPCWander.Enabled", false);
    _wanderDistance = sConfigMgr->GetOption<int16_t>("GuildEnclave.NPCWander.Distance", 0);

    _refundPercent = sConfigMgr->GetOption<float>("GuildEnclave.Refund", .8);
    if (_refundPercent > 1)
        _refundPercent = 1;

    _startingObjects.clear();
    std::string startingObjects = sConfigMgr->GetOption<std::string>("GuildEnclave.StartingObjects", "0");
    if (startingObjects != "0")
    {
        std::stringstream ss(startingObjects);
        std::string value;
        while (std::getline(ss, value, ','))
        {
            uint32_t catalogId = std::strtoul(value.c_str(), nullptr, 10);
            if (catalogId)
            {
                 LOG_INFO("server.loading", ">> GuildEnclaveConfig freecatalogsitems {}", catalogId);
                _startingObjects.push_back(catalogId);
            }
        }
    }
    
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

std::vector<uint32_t> GuildEnclaveConfig::GetStartingObjects() const
{
    return _startingObjects;
}
