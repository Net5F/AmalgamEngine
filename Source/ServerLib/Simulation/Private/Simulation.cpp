#include "Simulation.h"
#include "Network.h"
#include "ItemData.h"
#include "CastableData.h"
#include "EntityInitLua.h"
#include "EntityItemHandlerLua.h"
#include "ItemInitLua.h"
#include "DialogueLua.h"
#include "DialogueChoiceConditionLua.h"
#include "ISimulationExtension.h"
#include "Interaction.h"
#include "Inventory.h"
#include "SystemMessage.h"
#include "Log.h"
#include "Timer.h"
#include "tracy/Tracy.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace AM
{
namespace Server
{
Simulation::Simulation(Network& inNetwork, GraphicData& inGraphicData,
                       ItemData& inItemData, CastableData& castableData)
: network{inNetwork}
, entityInitLua{std::make_unique<EntityInitLua>()}
, entityItemHandlerLua{std::make_unique<EntityItemHandlerLua>()}
, itemInitLua{std::make_unique<ItemInitLua>()}
, dialogueLua{std::make_unique<DialogueLua>()}
, dialogueChoiceConditionLua{std::make_unique<DialogueChoiceConditionLua>()}
, world{*this,        inGraphicData,  inItemData,
        castableData, *entityInitLua, *itemInitLua}
, currentTick{0}
, engineLuaBindings{*entityInitLua,
                    *entityItemHandlerLua,
                    *itemInitLua,
                    *dialogueLua,
                    *dialogueChoiceConditionLua,
                    inGraphicData,
                    inItemData,
                    world,
                    network}
, extension{nullptr}
, clientConnectionSystem{*this, world, network, inGraphicData}
, nceLifetimeSystem{world, network}
, componentChangeSystem{world, network, inGraphicData}
, tileUpdateSystem{world, network}
, inputSystem{*this, world, network}
, movementSystem{world}
, aiSystem{world}
, castSystem{*this, network, inItemData, castableData}
, itemSystem{world, network, inItemData, *entityItemHandlerLua}
, inventorySystem{world, network, inItemData}
, dialogueSystem{world, network, *dialogueLua, *dialogueChoiceConditionLua}
, clientAOISystem{*this, world, network}
, movementSyncSystem{*this, world, network}
, componentSyncSystem{*this, world, network, inGraphicData}
, chunkStreamingSystem{world, network}
, scriptDataSystem{world, network, inItemData}
, saveSystem{*this, inItemData}
{
    // Register our current tick pointer with the classes that care.
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

Simulation::~Simulation() = default;

World& Simulation::getWorld()
{
    return world;
}

EntityInitLua& Simulation::getEntityInitLua()
{
    return *entityInitLua;
}

EntityItemHandlerLua& Simulation::getEntityItemHandlerLua()
{
    return *entityItemHandlerLua;
}

ItemInitLua& Simulation::getItemInitLua()
{
    return *itemInitLua;
}

DialogueLua& Simulation::getDialogueLua()
{
    return *dialogueLua;
}

DialogueChoiceConditionLua& Simulation::getDialogueChoiceConditionLua()
{
    return *dialogueChoiceConditionLua;
}

Uint32 Simulation::getCurrentTick() const
{
    return currentTick;
}

void Simulation::tick()
{
    ZoneScoped;

    /* Run all systems. */
    // Call the project's pre-everything logic.
    if (extension != nullptr) {
        extension->beforeAll();
    }

    // Process client connections and disconnections.
    clientConnectionSystem.processConnectionEvents();

    // Process requests to create or destroy non-client-controlled entities.
    nceLifetimeSystem.processUpdateRequests();

    // Process requests to change components.
    componentChangeSystem.processChangeRequests();

    // Receive and process tile update requests.
    tileUpdateSystem.updateTiles();

    // Call the project's pre-movement logic.
    if (extension != nullptr) {
        extension->afterMapAndConnectionUpdates();
    }

    // Send updated tile state to nearby clients.
    tileUpdateSystem.sendTileUpdates();

    // Receive and process client input messages.
    inputSystem.processInputMessages();

    // Move all of our entities.
    movementSystem.processMovements();

    // Run all of our AI.
    aiSystem.processAITick();

    // Process any cast requests and ongoing casts.
    castSystem.processCasts();

    // Process any waiting "use item" interaction messages.
    itemSystem.processUseItemInteractions();

    // Process and send item definition updates.
    itemSystem.processItemUpdates();

    // Process inventory updates.
    inventorySystem.processInventoryUpdates();

    // Process Talk interactions and dialogue choice requests, updating sim 
    // state and sending responses as necessary.
    dialogueSystem.processDialogueInteractions();

    // Call the project's post-sim-update logic.
    if (extension != nullptr) {
        extension->afterSimUpdate();
    }

    // Update each client entity's "entities in my AOI" list and send Init/
    // Delete messages.
    clientAOISystem.updateAOILists();

    // Send any updated entity movement state to nearby clients.
    movementSyncSystem.sendMovementUpdates();

    // Send initial Inventory state.
    inventorySystem.sendInventoryInits();

    // Send initial CastCooldown state.
    castSystem.sendCastCooldownInits();

    // Send any remaining updated entity component state to nearby clients.
    componentSyncSystem.sendUpdates();

    // Call the project's post-movement-sync logic.
    if (extension != nullptr) {
        extension->afterClientSync();
    }

    // Respond to chunk data requests.
    chunkStreamingSystem.sendChunks();

    // Respond to script data requests.
    scriptDataSystem.sendScripts();

    // If any category of data is due for saving, save it.
    saveSystem.saveIfNecessary();

    // Call the project's post-everything logic.
    if (extension != nullptr) {
        extension->afterAll();
    }

    currentTick++;

    FrameMark;
}

void Simulation::setExtension(std::unique_ptr<ISimulationExtension> inExtension)
{
    extension = std::move(inExtension);
    nceLifetimeSystem.setExtension(extension.get());
    tileUpdateSystem.setExtension(extension.get());
}

} // namespace Server
} // namespace AM
