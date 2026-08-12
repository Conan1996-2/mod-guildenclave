#include "GuildEnclaveWorld.h"

#include "GuildEnclaveConfig.h"
#include "GuildEnclaveCatalogMgr.h"
#include "GuildEnclaveMgr.h"

void GuildEnclaveWorldScript::OnAfterConfigLoad(bool)
{
    sGuildEnclaveConfig.Load();
}

void GuildEnclaveWorldScript::OnStartup()
{
    sGuildEnclaveCatalogMgr.Load();
    sGuildEnclaveMgr.Load();
}

void AddSC_GuildEnclaveWorld()
{
    new GuildEnclaveWorldScript();
}
