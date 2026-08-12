#include "ScriptMgr.h"


void AddSC_GuildEnclaveWorld();

void AddSC_GuildEnclaveBroker();
void AddSC_GuildEnclaveSalesman();
void AddSC_GuildEnclaveCommands();
void AddSC_GuildEnclaveNPCMovementScripts();
void AddSC_GuildEnclaveScripts();

void AddGuildEnclaveScripts()
{
    //
    // World
    //
    AddSC_GuildEnclaveWorld();

    //
    // Systems
    //
    AddSC_GuildEnclaveScripts();

    //
    // Gameplay
    //
    AddSC_GuildEnclaveCommands();
    AddSC_GuildEnclaveNPCMovementScripts();
    
    //
    // NPCs
    //
    AddSC_GuildEnclaveBroker();
    AddSC_GuildEnclaveSalesman();
}
