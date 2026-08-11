#include "GuildHouseWorld.h"

#include "GuildHouseConfig.h"
#include "GuildHouseCatalogMgr.h"
#include "GuildHouseMgr.h"

void GuildHouseWorldScript::OnAfterConfigLoad(bool)
{
    sGuildHouseConfig.Load();
}

void GuildHouseWorldScript::OnStartup()
{
    sGuildHouseCatalogMgr.Load();
    sGuildHouseMgr.Load();
}

void AddSC_GuildHouseWorld()
{
    new GuildHouseWorldScript();
}
