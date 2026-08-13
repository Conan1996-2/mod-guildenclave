#ifndef MOD_GUILDENCLAVE_SPAWNER_H
#define MOD_GUILDENCLAVE_SPAWNER_H

#include <cstdint>

class GuildEnclaveSpawner
{
public:

    static GuildEnclaveSpawner& Instance();

    // =====================================================
    // Startup
    // =====================================================
    void LoadPlacedAssets();

    // =====================================================
    // Spawn lookup
    // =====================================================
    bool HasExistingSpawn(uint32_t guildId, uint32_t assetId);

    // =====================================================
    // Asset spawning
    // =====================================================
    bool SpawnAsset(uint32_t guildId, uint32_t assetId, uint32_t catalogId, float x, float y, float z, float o, int w);
    void LoadPlacedAssets(uint32_t guildId);
    bool SpawnCreature(uint32_t guildId, uint32_t assetId, uint32_t phaseMask, uint32_t mapId, uint32_t entry, float x, float y, float z, float o, int w);

    // =====================================================
    // Removal
    // =====================================================
    bool RemoveAsset(uint32_t guildId, uint32_t assetId);
    bool RemoveAllAssets(uint32_t guildId);

private:

    GuildEnclaveSpawner() = default;

    bool SpawnGameObject(uint32_t guildId, uint32_t assetId, uint32_t phaseMask, uint32_t mapId, uint32_t entry, float x, float y, float z, float o);
    bool RemoveCreatureSpawn(uint32_t guid);
    bool RemoveGameObjectSpawn(uint32_t guid);
};

#define sGuildEnclaveSpawner GuildEnclaveSpawner::Instance()

#endif
