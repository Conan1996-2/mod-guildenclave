#ifndef MOD_GUILDENCLAVE_WORLD_H
#define MOD_GUILDENCLAVE_WORLD_H

#include "ScriptMgr.h"


class GuildEnclaveWorldScript : public WorldScript
{
public:

    GuildEnclaveWorldScript() : WorldScript("GuildEnclaveWorldScript")
    {
    }

    void OnAfterConfigLoad(bool reload) override;

    void OnStartup() override;
};

void AddSC_GuildEnclaveWorld();

#endif
