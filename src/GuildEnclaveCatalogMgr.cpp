#include "GuildEnclaveCatalogMgr.h"

#include "DatabaseEnv.h"
#include "QueryResult.h"
#include "Log.h"

#include <algorithm>

GuildEnclaveCatalogMgr& GuildEnclaveCatalogMgr::Instance()
{
    static GuildEnclaveCatalogMgr instance;
    return instance;
}

// =====================================================
// Load catalog database
// =====================================================
void GuildEnclaveCatalogMgr::Load()
{
    _categories.clear();
    _catalogs.clear();

    LOG_INFO("server.loading",">> Starting guildenclave_catagory");
    //
    // Categories
    //
    if (QueryResult result = WorldDatabase.Query("SELECT categoryId, parentId, name, sortOrder, enabled FROM guildenclave_category"))
    {
        do
        {
            Field* fields = result->Fetch();

            GHCategory category;
            category.Id = fields[0].Get<uint32_t>();
            category.ParentId = fields[1].Get<uint32_t>();
            category.Name = fields[2].Get<std::string>();
            category.SortOrder = fields[3].Get<uint16_t>();
            category.Enabled = fields[4].Get<bool>();

            _categories.emplace(category.Id, category);

        } while (result->NextRow());
    }

    LOG_INFO("server.loading",">> Starting guildenclave_catalog");
    //
    // Catalog Items
    //
    if (QueryResult result = WorldDatabase.Query("SELECT catalogId, categoryId, name, price, spawnFlags, behaviorFlags, enabled FROM guildenclave_catalog"))
    {
        do
        {
            Field* fields = result->Fetch();

            GHCatalog catalog;
            catalog.CatalogId = fields[0].Get<uint32_t>();
            catalog.CategoryId = fields[1].Get<uint32_t>();
            catalog.Name = fields[2].Get<std::string>();
            catalog.Price  = fields[3].Get<uint64_t>();
            catalog.SpawnFlags = static_cast<GHSpawnFlags>(fields[4].Get<uint32_t>());
            catalog.BehaviorFlags = static_cast<GHBehaviorFlags>(fields[5].Get<uint32_t>());
            catalog.Enabled = fields[6].Get<bool>();

            _catalogs.emplace(catalog.CatalogId, catalog);

        } while (result->NextRow());
    }

    LOG_INFO("server.loading",">> Starting guildenclave_catalog_asset");
    //
    // Catalog Components
    //
    uint16_t componentCount = 0;
//    if (QueryResult result = WorldDatabase.Query("SELECT componentId,catalogId,spawnFlags,behaviorFlags,entryId,displayId,scale,scriptType,scriptData,xOffset,yOffset,zOffset,oOffset,targetMapId,targetX,targetY,targetZ,targetO,childCatalogId,sortOrder "
    if (QueryResult result = WorldDatabase.Query("SELECT componentId,catalogId,spawnFlags,behaviorFlags,entryId,displayId,scale,xOffset,yOffset,zOffset,oOffset,childCatalogId,sortOrder "
        "FROM guildenclave_catalog_asset ORDER BY sortOrder"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32_t catalogId = fields[1].Get<uint32_t>();

            auto catalogItr = _catalogs.find(catalogId);
            if (catalogItr == _catalogs.end())
                continue;

            GHCatalogAsset component;
            component.ComponentId = fields[0].Get<uint32_t>();
            component.CatalogId = catalogId;
            component.SpawnFlags = static_cast<GHSpawnFlags>(fields[2].Get<uint32_t>());
            component.BehaviorFlags = static_cast<GHBehaviorFlags>(fields[3].Get<uint32_t>());
            component.Entry = fields[4].Get<uint32_t>();
            component.DisplayId = fields[5].Get<uint32_t>();
            component.Scale = fields[6].Get<float>();
            component.XOffset = fields[7].Get<float>();
            component.YOffset = fields[8].Get<float>();
            component.ZOffset = fields[9].Get<float>();
            component.OOffset = fields[10].Get<float>();
/*
            component.ScriptType = static_cast<GHScriptType>(fields[7].Get<uint32_t>());
            component.ScriptData = fields[8].IsNull() ? "" : fields[8].Get<std::string>();
            component.TargetMap = fields[13].IsNull() ? 0 : fields[13].Get<uint32_t>();
            component.TargetX = fields[14].IsNull() ? 0.0f : fields[14].Get<float>();
            component.TargetY = fields[15].IsNull() ? 0.0f : fields[15].Get<float>();
            component.TargetZ = fields[16].IsNull() ? 0.0f : fields[16].Get<float>();
            component.TargetO = fields[17].IsNull() ? 0.0f : fields[17].Get<float>();
*/
            component.ChildCatalogId = fields[11].IsNull() ? 0 : fields[11].Get<uint32_t>();
            component.SortOrder = fields[12].Get<uint16_t>();

            //
            // Store component in parent catalog
            //
            catalogItr->second.Components.push_back(component);
            componentCount++;

        } while (result->NextRow());
    }

    LOG_INFO("server.loading",">> GuildEnclaveCatalogMgr loaded {} categories, {} catalogs, {} components", _categories.size(), _catalogs.size(), componentCount);
}

const GHCatalog* GuildEnclaveCatalogMgr::GetCatalog(uint32_t catalogId) const
{
    auto itr = _catalogs.find(catalogId);
    if (itr == _catalogs.end())
        return nullptr;

    return &itr->second;
}

const GHCategory* GuildEnclaveCatalogMgr::GetCategory(uint32_t categoryId) const
{
    auto itr = _categories.find(categoryId);
    if (itr == _categories.end())
        return nullptr;

    return &itr->second;
}

// =====================================================
// Root category list
// =====================================================
std::vector<const GHCategory*> GuildEnclaveCatalogMgr::GetRootCategories() const
{
    std::vector<const GHCategory*> result;

    for (auto const& [id, category] : _categories)
    {
        if (category.ParentId == 0 && category.Enabled)
            result.push_back(&category);
    }

    std::sort(result.begin(), result.end(), [](const GHCategory* a, const GHCategory* b) { return a->SortOrder < b->SortOrder; });

    return result;
}

// =====================================================
// Child category list
// =====================================================
std::vector<const GHCategory*> GuildEnclaveCatalogMgr::GetChildCategories(uint32_t parentId) const
{
    std::vector<const GHCategory*> result;

    for (auto const& [id, category] : _categories)
    {
        if (category.ParentId == parentId && category.Enabled)
            result.push_back(&category);
    }

    std::sort(result.begin(), result.end(), [](const GHCategory* a, const GHCategory* b) { return a->SortOrder < b->SortOrder; });

    return result;
}

// =====================================================
// Catalog list by category
// =====================================================
std::vector<const GHCatalog*> GuildEnclaveCatalogMgr::GetCatalogs(uint32_t categoryId, TeamId team) const
{
    std::vector<const GHCatalog*> result;

    for (auto const& [id, catalog] : _catalogs)
    {
        if (!catalog.Enabled || catalog.CategoryId != categoryId)
            continue;

        if (GuildEnclaveUtil::HasFlag(catalog.BehaviorFlags, GH_FACTION_NEUTRAL) ||
            (team == TEAM_ALLIANCE && GuildEnclaveUtil::HasFlag(catalog.BehaviorFlags, GH_FACTION_ALLIANCE)) ||
            (team == TEAM_HORDE && GuildEnclaveUtil::HasFlag(catalog.BehaviorFlags, GH_FACTION_HORDE)))
                result.push_back(&catalog);
    }

    std::sort(result.begin(), result.end(), [](const GHCatalog* a, const GHCatalog* b) { return a->Name < b->Name; });

    return result;
}

// =====================================================
// All catalogs
// =====================================================
std::vector<const GHCatalog*> GuildEnclaveCatalogMgr::GetAllCatalogs(TeamId team) const
{
    std::vector<const GHCatalog*> result;

    for (auto const& [id, catalog] : _catalogs)
    {
        if (!catalog.Enabled)
            continue;        
 
        if (GuildEnclaveUtil::HasFlag(catalog.BehaviorFlags, GH_FACTION_NEUTRAL) || (team == TEAM_ALLIANCE && GuildEnclaveUtil::HasFlag(catalog.BehaviorFlags, GH_FACTION_ALLIANCE)) || (team == TEAM_HORDE && GuildEnclaveUtil::HasFlag(catalog.BehaviorFlags, GH_FACTION_HORDE)))
                result.push_back(&catalog);
    }

    std::sort(result.begin(), result.end(), [](const GHCatalog* a, const GHCatalog* b) { return a->Name < b->Name; });

    return result;
}
