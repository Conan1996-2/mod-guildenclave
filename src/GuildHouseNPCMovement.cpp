#include "GuildHouseNPCMovement.h"

#include "Creature.h"
#include "MotionMaster.h"
#include "GuildHouseMgr.h"

GuildHouseNPCMovement::GuildHouseNPCMovement() : PlayerScript("GuildHouseNPCMovement")
{
}

void GuildHouseNPCMovement::OnCreatureInteraction(Player* player, Creature* creature)
{
    if (!creature)
        return;

    uint32 spawnId = creature->GetSpawnId();

//    if (!sGuildHouseMgr.IsGuildHouseCreature(spawnId))
//        return;

    creature->StopMoving();
    creature->GetMotionMaster()->MoveIdle();

    creature->m_Events.AddEvent(new ResumeGuildHouseMovementEvent(creature), creature->m_Events.CalculateTime(30000));
}

ResumeGuildHouseMovementEvent::ResumeGuildHouseMovementEvent(Creature* creature) : _creature(creature)
{
}

bool ResumeGuildHouseMovementEvent::Execute(uint64 /*time*/, uint32 /*diff*/)
{
    if (_creature && _creature->IsInWorld())
        _creature->GetMotionMaster()->Initialize();

    return true;
}

void AddSC_GuildHouseNPCMovementScripts()
{
   new GuildHouseNPCMovement();
}
