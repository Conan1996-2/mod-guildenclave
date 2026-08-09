#include "GuildHousePhaseMgr.h"

#include "DatabaseEnv.h"
#include "Player.h"
#include "Log.h"

#include <algorithm>

GuildHousePhaseMgr& GuildHousePhaseMgr::Instance()
{
    static GuildHousePhaseMgr instance;
    return instance;
}

// =====================================================
// Load
// =====================================================
void GuildHousePhaseMgr::Load()
{
    _phases.clear();

    QueryResult result = CharacterDatabase.Query("SELECT gp.guildId,gp.phaseMask,gh.locationId FROM guildhouse_phase gp INNER JOIN guildhouse gh ON gp.guildId = gh.guildId");
    if (!result)
    {
        LOG_INFO("server.loading", "No Guild House phases found");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        GHPhaseRecord phase;
        phase.GuildId = fields[0].Get<uint32>();
        phase.PhaseMask = fields[1].Get<uint32>();

        uint32 locationId = fields[2].Get<uint32>();
        if (QueryResult location = WorldDatabase.Query("SELECT mapId,positionX,positionY,positionZ,orientation,minX,maxX,minY,maxY FROM guildhouse_locations WHERE id={}", locationId))
        {
            Field* loc = location->Fetch();

            phase.MapId = loc[0].Get<uint32>();
            phase.X = loc[1].Get<float>();
            phase.Y = loc[2].Get<float>();
            phase.Z = loc[3].Get<float>();
            phase.O = loc[4].Get<float>();
            phase.MinX = loc[5].Get<float>();
            phase.MaxX = loc[6].Get<float>();
            phase.MinY = loc[7].Get<float>();
            phase.MaxY = loc[8].Get<float>();
        }
        else
        {
            LOG_ERROR("server.loading", "GuildHousePhaseMgr: Missing guildhouse location {} for guild {}", locationId, phase.GuildId);
            continue;
        }
        
        CharacterDatabase.Execute("UPDATE guildhouse_phase SET activeMembers=0 WHERE guildId={}", phase.GuildId);

        _phases.emplace(phase.GuildId, phase);
    } while(result->NextRow());

    LOG_INFO("server.loading", "Loaded {} Guild House phases", _phases.size());
}

// =====================================================
// Create Phase
// =====================================================

uint32_t GuildHousePhaseMgr::CreatePhase(uint32_t guildId, uint32_t locationId)
{
    if (HasPhase(guildId))
    {
        return GetPhaseMask(guildId);
    }

    QueryResult result = WorldDatabase.Query("SELECT mapId,positionX,positionY,positionZ,orientation,minX,maxX,minY,maxY FROM guildhouse_locations WHERE id={}", locationId);
    if (!result)
        return 0;

    Field* fields = result->Fetch();

    uint32_t phaseMask = GeneratePhaseMask(locationId);
    if (!phaseMask)
        return 0;

    GHPhaseRecord phase;
    phase.GuildId = guildId;
    phase.PhaseMask = phaseMask;
    phase.MapId = fields[0].Get<uint32>();
    phase.X = fields[1].Get<float>();
    phase.Y = fields[2].Get<float>();
    phase.Z = fields[3].Get<float>();
    phase.O = fields[4].Get<float>();
    phase.MinX = fields[5].Get<float>();
    phase.MaxX = fields[6].Get<float>();
    phase.MinY = fields[7].Get<float>();
    phase.MaxY = fields[8].Get<float>();

    _phases.emplace(guildId, phase);

    CharacterDatabase.Execute("INSERT INTO guildhouse_phase (guildId,phaseMask) VALUES ({},{})", phase.GuildId, phase.PhaseMask);

    LOG_INFO("module", "Created Guild House phase {} for guild {}", phaseMask, guildId);

    return phaseMask;
}

// =====================================================
// Enter
// =====================================================
bool GuildHousePhaseMgr::EnterPhase(Player* player, uint32_t guildId)
{
    if (!player)
        return false;

    const GHPhaseRecord* phase = GetPhase(guildId);
    if (!phase)
        return false;

    AddMember(guildId, player->GetGUID().GetCounter());

    player->TeleportTo(phase->MapId, phase->X, phase->Y, phase->Z, phase->O);
    player->SetPhaseMask(phase->PhaseMask, true);
    return true;
}

// =====================================================
// Leave
// =====================================================
bool GuildHousePhaseMgr::LeavePhase(Player* player)
{
    if (!player)
        return false;

    RemoveMember(player->GetGuildId(), player->GetGUID().GetCounter());

    player->SetPhaseMask(1, true);
    player->SetRestFlag(REST_FLAG_IN_CITY);
    return true;
}

// =====================================================
// Member tracking
// =====================================================
bool GuildHousePhaseMgr::AddMember(uint32_t guildId, uint64_t guid)
{
    LOG_INFO("server.loading", "Addmember enter");

    auto itr = _phases.find(guildId);
    if (itr == _phases.end())
        return false;

    itr->second.Members.insert(guid);
    
    uint32_t activeMembers = static_cast<uint32_t>(itr->second.Members.size());
    CharacterDatabase.Execute( "UPDATE guildhouse_phase SET activeMembers={} WHERE guildId={}", activeMembers, guildId);
    
    LOG_INFO("server.loading", "Addmember to guild phase. Count: {}", activeMembers);
    
    return true;
}

bool GuildHousePhaseMgr::RemoveMember(uint32_t guildId, uint64_t guid)
{
    LOG_INFO("server.loading", "Removemember enter");

    auto itr = _phases.find(guildId);
    if (itr == _phases.end())
        return false;

    itr->second.Members.erase(guid);

    uint32_t activeMembers = static_cast<uint32_t>(itr->second.Members.size());
    CharacterDatabase.Execute("UPDATE guildhouse_phase SET activeMembers={} WHERE guildId={}", activeMembers, guildId);
    
    LOG_INFO("server.loading", "Removemember from guild phase. Count: {}", activeMembers);
    
    return true;
}

// =====================================================
// Boundary
// =====================================================
bool GuildHousePhaseMgr::CheckBoundary(Player* player)
{
    if (!IsMember(player))
        return true;

    const GHPhaseRecord* phase = GetPhase(player->GetGuildId());
    if (!phase)
        return true;

    float x = player->GetPositionX();
    float y = player->GetPositionY();

    if (x < phase->MinX || x > phase->MaxX || y < phase->MinY || y > phase->MaxY)
    {
        player->TeleportTo(phase->MapId, phase->X, phase->Y, phase->Z, phase->O);
        return false;
    }

    return true;
}

// =====================================================
// Remove
// =====================================================
bool GuildHousePhaseMgr::RemovePhase(uint32_t guildId)
{
    auto itr = _phases.find(guildId);
    if (itr == _phases.end())
        return false;

    _phases.erase(itr);

    CharacterDatabase.Execute("DELETE FROM guildhouse_phase WHERE guildId={}", guildId);
    return true;
}

// =====================================================
// Lookup
// =====================================================
bool GuildHousePhaseMgr::HasPhase(uint32_t guildId) const
{
    return _phases.find(guildId) != _phases.end();
}

uint32_t GuildHousePhaseMgr::GetPhaseMask(uint32_t guildId) const
{
    auto itr = _phases.find(guildId);
    if (itr == _phases.end())
        return 0;

    return itr->second.PhaseMask;
}

const GHPhaseRecord* GuildHousePhaseMgr::GetPhase(uint32_t guildId) const
{
    auto itr = _phases.find(guildId);
    if (itr == _phases.end())
        return nullptr;

    return &itr->second;
}

bool GuildHousePhaseMgr::IsMember(Player* player) const
{
    if (!player)
        return false;

    auto itr = _phases.find(player->GetGuildId());
    if (itr == _phases.end())
        return false;

    return itr->second.Members.find(player->GetGUID().GetCounter()) != itr->second.Members.end();
}

// =====================================================
// Phase generator
// =====================================================
uint32_t GuildHousePhaseMgr::GeneratePhaseMask(uint32_t locationId)
{
    std::unordered_set<uint32_t> usedMasks;

    QueryResult result = CharacterDatabase.Query("SELECT gp.phaseMask FROM guildhouse_phase gp INNER JOIN guildhouse gh ON gp.guildId = gh.guildId WHERE gh.locationId = {}", locationId);
    if (result)
    {
        do
        {
            usedMasks.insert(result->Fetch()[0].Get<uint32>());
        }
        while (result->NextRow());
    }

    for (uint32_t mask = 2; mask <= (1u << 30); mask <<= 1)
    {
        if (usedMasks.find(mask) == usedMasks.end())
        {
            LOG_INFO("server.loading", "available phase masks for location {}: {}", locationId, mask);
            return mask;
        }
    }

    LOG_INFO("server.loading", "No available phase masks for location {}", locationId);
    return 0;
}
