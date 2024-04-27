#include "Simulation.h"
#include "Network.h"
#include "EntityInitLua.h"
#include "EntityItemHandlerLua.h"
#include "ItemInitLua.h"
#include "DialogueLua.h"
#include "DialogueChoiceConditionLua.h"
#include "EnttGroups.h"
#include "ISimulationExtension.h"
#include "EntityInteractionRequest.h"
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
Simulation::Simulation(Network& inNetwork, GraphicData& inGraphicData)
: network{inNetwork}
, entityInitLua{std::make_unique<EntityInitLua>()}
, entityItemHandlerLua{std::make_unique<EntityItemHandlerLua>()}
, itemInitLua{std::make_unique<ItemInitLua>()}
, dialogueLua{std::make_unique<DialogueLua>()}
, dialogueChoiceConditionLua{std::make_unique<DialogueChoiceConditionLua>()}
, world{inGraphicData, *entityInitLua, *itemInitLua}
, currentTick{0}
, engineLuaBindings{*entityInitLua, *entityItemHandlerLua,       *itemInitLua,
                    *dialogueLua,   *dialogueChoiceConditionLua, world,
                    network}
, extension{nullptr}
, entityInteractionRequestQueue{inNetwork.getEventDispatcher()}
, itemInteractionRequestQueue{inNetwork.getEventDispatcher()}
, entityInteractionQueueMap{}
, itemInteractionQueueMap{}
, clientConnectionSystem{*this, world, network, inGraphicData}
, nceLifetimeSystem{world, network}
, componentChangeSystem{world, network, inGraphicData}
, tileUpdateSystem{world, network}
, inputSystem{*this, world, network}
, movementSystem{world}
, aiSystem{world}
, itemSystem{*this, network, *entityItemHandlerLua}
, inventorySystem{world, network}
, dialogueSystem{*this, network, *dialogueLua, *dialogueChoiceConditionLua}
, clientAOISystem{*this, world, network}
, movementSyncSystem{*this, world, network}
, componentSyncSystem{*this, world, network, inGraphicData}
, chunkStreamingSystem{world, network}
, scriptDataSystem{world, network}
, saveSystem{world}
{
    // Initialize our entt groups.
    EnttGroups::init(world.registry);

    // Initialize the Lua environments and add our bindings.
    entityInitLua->luaState.open_libraries(sol::lib::base);
    entityItemHandlerLua->luaState.open_libraries(sol::lib::base);
    itemInitLua->luaState.open_libraries(sol::lib::base);
    dialogueLua->luaState.open_libraries(sol::lib::base);
    dialogueChoiceConditionLua->luaState.open_libraries(sol::lib::base);
    engineLuaBindings.addBindings();

    // Register our current tick pointer with the classes that care.
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

Simulation::~Simulation() = default;

bool Simulation::popEntityInteractionRequest(
    EntityInteractionType interactionType, EntityInteractionData& data)
{
    // If there's a waiting message of the given type, validate it.
    std::queue<EntityInteractionRequest>& queue{
        entityInteractionQueueMap[interactionType]};
    if (!(queue.empty())) {
        EntityInteractionRequest interactionRequest{queue.front()};
        queue.pop();
        entt::registry& registry{world.registry};

        // Find the client's entity ID.
        auto it{world.netIDMap.find(interactionRequest.netID)};
        if (it == world.netIDMap.end()) {
            // Client doesn't exist (may have disconnected), do nothing.
            return false;
        }
        entt::entity clientEntity{it->second};
        entt::entity targetEntity{interactionRequest.targetEntity};

        // Check that the target exists.
        if (!(registry.valid(targetEntity))) {
            return false;
        }

        // Check that the client is in range of the target.
        const Position& clientPosition{registry.get<Position>(clientEntity)};
        const Position& targetPosition{registry.get<Position>(targetEntity)};
        if (clientPosition.squaredDistanceTo(targetPosition)
            > SharedConfig::SQUARED_INTERACTION_DISTANCE) {
            network.serializeAndSend(
                interactionRequest.netID,
                SystemMessage{"You must move closer to interact with that."});
            return false;
        }

        // Check that the target actually has this interaction type.
        if (auto* interaction{registry.try_get<Interaction>(targetEntity)};
            !interaction
            || !(interaction->supports(interactionRequest.interactionType))) {
            return false;
        }

        // Request is valid. Return it.
        data.clientEntity = clientEntity;
        data.targetEntity = targetEntity;
        data.clientID = interactionRequest.netID;
        return true;
    }

    return false;
}

bool Simulation::popItemInteractionRequest(ItemInteractionType interactionType,
                                           ItemInteractionData& data)
{
    // If there's a waiting message of the given type, validate it.
    std::queue<ItemInteractionRequest>& queue{
        itemInteractionQueueMap[interactionType]};
    if (!(queue.empty())) {
        ItemInteractionRequest interactionRequest{queue.front()};
        queue.pop();

        // Find the client's entity ID.
        auto it{world.netIDMap.find(interactionRequest.netID)};
        if (it == world.netIDMap.end()) {
            // Client doesn't exist (may have disconnected), do nothing.
            return false;
        }
        entt::entity clientEntity{it->second};

        // Check that the item exists and actually has this interaction type.
        const auto& inventory{world.registry.get<Inventory>(clientEntity)};
        const Item* item{
            inventory.getItem(interactionRequest.slotIndex, world.itemData)};
        if (!item
            || !(item->supportsInteraction(
                interactionRequest.interactionType))) {
            return false;
        }

        // Request is valid. Return it.
        data.clientEntity = clientEntity;
        data.slotIndex = interactionRequest.slotIndex;
        data.clientID = interactionRequest.netID;
        data.item = item;
        return true;
    }

    return false;
}

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

Uint32 Simulation::getCurrentTick()
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

    // Sort any waiting interaction messages into their type-based queues.
    sortInteractionMessages();

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

    // Process any waiting item interaction messages.
    itemSystem.processItemInteractions();

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

void Simulation::sortInteractionMessages()
{
    // Push each message into the associated queue, based on its type enum.
    EntityInteractionRequest entityInteractionRequest{};
    while (entityInteractionRequestQueue.pop(entityInteractionRequest)) {
        entityInteractionQueueMap[entityInteractionRequest.interactionType]
            .push(entityInteractionRequest);
    }

    ItemInteractionRequest itemInteractionRequest{};
    while (itemInteractionRequestQueue.pop(itemInteractionRequest)) {
        itemInteractionQueueMap[itemInteractionRequest.interactionType].push(
            itemInteractionRequest);
    }
}

} // namespace Server
} // namespace AM
