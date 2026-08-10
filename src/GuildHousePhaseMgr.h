/*
#ifndef MOD_GUILDHOUSE_PHASE_MGR_H
#define MOD_GUILDHOUSE_PHASE_MGR_H

#include <cstdint>
#include <unordered_map>

#include "GuildHouseTypes.h"

class Player;

class GuildHousePhaseMgr
{
public:

    static GuildHousePhaseMgr& Instance();

    // =====================================================
    // Lifecycle
    // =====================================================
    void Load();
    uint32_t CreatePhase(uint32_t guildId, uint32_t locationId);
    bool RemovePhase(uint32_t guildId);

    // =====================================================
    // Player phase handling
    // =====================================================
    bool EnterPhase(Player* player, uint32_t guildId);
    bool LeavePhase(Player* player);
    bool IsMember(Player* player) const;
    bool CheckBoundary(Player* player);

    // =====================================================
    // Lookup
    // =====================================================
    bool HasPhase(uint32_t guildId) const;
    uint32_t GetPhaseMask(uint32_t guildId) const;
    const GHGuildHouse* GetPhase(uint32_t guildId) const;

private:

    GuildHousePhaseMgr() = default;

    uint32_t GeneratePhaseMask(uint32_t locationId);
    bool AddMember(uint32_t guildId, uint64_t guid);
    bool RemoveMember(uint32_t guildId, uint64_t guid);

private:

};

#define sGuildHousePhaseMgr GuildHousePhaseMgr::Instance()

#endif
*/
