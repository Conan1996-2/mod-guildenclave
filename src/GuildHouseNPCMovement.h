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
   // new GuildHouseNPCMovement();
}
