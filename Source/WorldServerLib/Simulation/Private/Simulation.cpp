#include "Simulation.h"
#include "SimulationContext.h"
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
namespace WorldServer
{
Simulation::Simulation(const SimulationContext& inSimContext)
: network{inSimContext.network}
, entityInitLua{std::make_unique<EntityInitLua>()}
, entityItemHandlerLua{std::make_unique<EntityItemHandlerLua>()}
, itemInitLua{std::make_unique<ItemInitLua>()}
, dialogueLua{std::make_unique<DialogueLua>()}
, dialogueChoiceConditionLua{std::make_unique<DialogueChoiceConditionLua>()}
, world{inSimContext}
, currentTick{0}
, engineLuaBindings{*entityInitLua,
                    *entityItemHandlerLua,
                    *itemInitLua,
                    *dialogueLua,
                    *dialogueChoiceConditionLua,
                    inSimContext.graphicData,
                    inSimContext.itemData,
                    world,
                    network}
, extension{nullptr}
, clientConnectionSystem{inSimContext}
, nonPlayerEntityLifetimeSystem{inSimContext}
, componentChangeSystem{inSimContext}
, tileUpdateSystem{inSimContext}
, inputSystem{inSimContext}
, aiSystem{inSimContext}
, movementSystem{inSimContext}
, castSystem{inSimContext}
, itemSystem{inSimContext}
, inventorySystem{inSimContext}
, dialogueSystem{inSimContext}
, clientAOISystem{inSimContext}
, movementSyncSystem{inSimContext}
, componentSyncSystem{inSimContext}
, chunkStreamingSystem{inSimContext}
, scriptDataSystem{inSimContext}
, chatPositionSyncSystem{inSimContext}
, saveSystem{inSimContext}
{
    // Register our current tick pointer with the classes that care.
    Log::registerCurrentTickPtr(&currentTick);
    network.clientEndpoint.registerCurrentTickPtr(&currentTick);

    // Load the saved world state.
    world.load();
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
    extension->beforeAll();

    // Process client connections and disconnections.
    clientConnectionSystem.processConnectionEvents();

    // Process requests to create or destroy non-player entities.
    nonPlayerEntityLifetimeSystem.processUpdateRequests();

    // Process requests to change components.
    componentChangeSystem.processChangeRequests();

    // Receive and process tile update requests.
    tileUpdateSystem.updateTiles();

    // Call the project's pre-movement logic.
    extension->afterMapAndConnectionUpdates();

    // Send updated tile state to nearby clients.
    tileUpdateSystem.sendTileUpdates();

    // Receive and process client input messages.
    inputSystem.processInputMessages();

    // Update the nav grid to reflect any collision changes, so AI sees the
    // current state of the world.
    world.navGrid.update();

    // Run all of our AI.
    aiSystem.processAITick();

    // Move all of our entities.
    movementSystem.processMovements();

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
    extension->afterSimUpdate();

    // Update each player entity's "entities in my AOI" list and send Init/
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
    extension->afterClientSync();

    // Respond to chunk data requests.
    chunkStreamingSystem.sendChunks();

    // Respond to script data requests.
    scriptDataSystem.sendScripts();

    // Tell the chat server where player entities are, for routing local
    // chat.
    chatPositionSyncSystem.sendPositionUpdates();

    // If any category of data is due for saving, save it.
    saveSystem.saveIfNecessary();

    // Send any queued AccountData saves to the account server.
    world.accountSaveQueue.processSaves();

    // Call the project's post-everything logic.
    extension->afterAll();

    currentTick++;

    FrameMark;
}

void Simulation::shutdown()
{
    // Save everything (including lingering player entities) and back it up.
    // Note: This also queues each player entity's AccountData.
    saveSystem.saveForShutdown();

    // Wait for the AccountData saves (including any that were already
    // queued, e.g. from recent logouts) to be confirmed.
    world.accountSaveQueue.flush(SHUTDOWN_ACCOUNT_SAVE_TIMEOUT);
}

void Simulation::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
    clientConnectionSystem.setExtension(extension);
    nonPlayerEntityLifetimeSystem.setExtension(extension);
    componentChangeSystem.setExtension(extension);
    tileUpdateSystem.setExtension(extension);
    itemSystem.setExtension(extension);
}

} // namespace WorldServer
} // namespace AM
