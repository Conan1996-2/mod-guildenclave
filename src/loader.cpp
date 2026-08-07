#include "ScriptMgr.h"


void AddSC_GuildHouseWorld();

void AddSC_GuildHouseBroker();
void AddSC_GuildHouseSalesman();
void AddSC_GuildHouseCommands();
void AddSC_GuildHouseScripts();

void AddGuildHouseScripts()
{
    //
    // World
    //
    AddSC_GuildHouseWorld();

    //
    // Systems
    //
    AddSC_GuildHouseScripts();

    //
    // Gameplay
    //
    AddSC_GuildHouseCommands();
    //AddSC_GuildHouseNPCMovementScripts();
    
    //
    // NPCs
    //
    AddSC_GuildHouseBroker();
    AddSC_GuildHouseSalesman();
}
