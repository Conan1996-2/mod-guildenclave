#ifndef GUILDENCLAVE_NPC_MOVEMENT_H
#define GUILDENCLAVE_NPC_MOVEMENT_H

#include "ScriptMgr.h"

class Player;
class Creature;

class ResumeCreatureMovementEvent : public BasicEvent
{
public:
    explicit ResumeCreatureMovementEvent(Creature* creature);

    bool Execute(uint64 /*time*/, uint32 /*diff*/) override;

private:
    Creature* _creature;
};

/*
class GuildEnclaveNPCMovement : public PlayerScript
{
public:
    GuildEnclaveNPCMovement();

    void OnCreatureInteraction(Player* player, Creature* creature) override;
};
*/

void AddSC_GuildEnclaveNPCMovementScripts();

#endif
