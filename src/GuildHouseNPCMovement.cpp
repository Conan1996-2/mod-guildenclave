#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "MotionMaster.h"
#include "GuildHouseMgr.h"

class GuildHouseNPCMovement : public PlayerScript
{
public:
    GuildHouseNPCMovement() : PlayerScript("GuildHouseNPCMovement") {}

    void OnCreatureInteraction(Player* player, Creature* creature) override
    {
        if (!creature)
            return;

        uint32 spawnId = creature->GetSpawnId();

        // Check if this is one of your guild house NPCs
        //if (!sGuildHouseMgr.IsGuildHouseCreature(spawnId))
        //    return;

        // Stop wandering
        creature->StopMoving();
        creature->GetMotionMaster()->MoveIdle();

        // Resume after 30 seconds
        creature->m_Events.AddEvent(new ResumeCreatureMovementEvent(creature), creature->m_Events.CalculateTime(30000));
    }
};

class ResumeCreatureMovementEvent : public BasicEvent
{
public:
    explicit ResumeCreatureMovementEvent(Creature* creature) : _creature(creature) {}

    bool Execute(uint64 /*time*/, uint32 /*diff*/) override
    {
        if (_creature && _creature->IsInWorld())
        {
            _creature->GetMotionMaster()->Initialize();
        }

        return true;
    }

private:
    Creature* _creature;
};

void AddSC_GuildHouseNPCMovementScripts()
{
   new GuildHouseNPCMovement();
}
