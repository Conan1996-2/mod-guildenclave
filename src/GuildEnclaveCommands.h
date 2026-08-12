#ifndef MOD_GUILDENCLAVE_COMMANDS_H
#define MOD_GUILDENCLAVE_COMMANDS_H

#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

class GuildEnclaveCommandScript : public CommandScript
{
public:

    GuildEnclaveCommandScript();

    ChatCommandTable GetCommands() const override;

    // =====================================================
    // NPC MANAGEMENT
    // =====================================================
    static bool HandleAddBroker(ChatHandler* handler);
    static bool HandleAddSalesman(ChatHandler* handler);

    // =====================================================
    // GUILD Enclave MANAGEMENT
    // =====================================================
    static bool HandleSellGuildEnclave(ChatHandler* handler, char const* args);
    static bool HandleTeleportGuildEnclave(ChatHandler* handler, char const* args);

    // =====================================================
    // ASSET MANAGEMENT
    // =====================================================
    static bool HandleListAssets(ChatHandler* handler, char const* args);
    static bool HandlePlaceAsset(ChatHandler* handler, char const* args);
    static bool HandleMoveAsset(ChatHandler* handler, char const* args);
    static bool HandleStoreAsset(ChatHandler* handler, char const* args);
    static bool HandleSellAsset(ChatHandler* handler, char const* args);

    // =====================================================
    // CATALOG / SHOP
    // =====================================================
    static bool HandleListCategories(ChatHandler* handler, char const* args);
    static bool HandleListCatalog(ChatHandler* handler, char const* args);
    static bool HandlePurchaseCatalog(ChatHandler* handler, char const* args);
};

// =====================================================
// Script registration
// =====================================================
void AddSC_GuildEnclaveCommands();

#endif
