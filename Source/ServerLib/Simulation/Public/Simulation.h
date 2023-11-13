#pragma once

#include "EntityInteractionType.h"
#include "ItemInteractionType.h"
#include "World.h"
#include "EngineLuaBindings.h"
#include "EntityInteractionRequest.h"
#include "ItemInteractionRequest.h"
#include "ClientConnectionSystem.h"
#include "NceLifetimeSystem.h"
#include "ComponentChangeSystem.h"
#include "TileUpdateSystem.h"
#include "InputSystem.h"
#include "MovementSystem.h"
#include "AISystem.h"
#include "ItemSystem.h"
#include "InventorySystem.h"
#include "ClientAOISystem.h"
#include "MovementSyncSystem.h"
#include "ComponentSyncSystem.h"
#include "ChunkStreamingSystem.h"
#include "ScriptDataSystem.h"
#include "MapSaveSystem.h"
#include "QueuedEvents.h"
#include <SDL_stdinc.h>
#include <atomic>
#include <queue>
#include <memory>

namespace sol
{
class state;
}

namespace AM
{
namespace Server
{
class Network;
class SpriteData;
class ISimulationExtension;

/**
 * Manages the simulation, including world state and system processing.
 *
 * The simulation is built on an ECS architecture:
 *   Entities exist in a registry, owned by the World class.
 *   Components that hold data are attached to each entity.
 *   Systems that act on sets of components are owned and ran by this class.
 */
class Simulation
{
public:
    /** An unreasonable amount of time for the sim tick to be late by. */
    static constexpr double SIM_DELAYED_TIME_S{.001};

    Simulation(Network& inNetwork, SpriteData& inSpriteData);

    ~Simulation();

    /** 
     * Registers the given queue to receive interaction events of a 
     * particular type.
     *
     * Interaction events occur when the user left-clicks an entity, or right-
     * clicks and selects an interaction from the menu.
     *
     * @param interactionType The type of interaction. Should be cast from an 
     *                        InteractionType.
     * @param queue The queue to register.
     *
     * Note: Only 1 queue can be subscribed to each type of interaction.
     */
    void registerInteractionQueue(EntityInteractionType interactionType,
                                  std::queue<EntityInteractionRequest>& queue);
    void registerInteractionQueue(ItemInteractionType interactionType,
                                  std::queue<ItemInteractionRequest>& queue);

    /**
     * Returns a reference to the simulation's world state.
     */
    World& getWorld();

    /**
     * Returns a reference to the simulation's Lua engine.
     */
    sol::state& getLua();

    /**
     * Returns the simulation's current tick number.
     */
    Uint32 getCurrentTick();

    /**
     * Updates accumulatedTime. If greater than the tick timestep, processes
     * the next sim iteration.
     */
    void tick();

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<ISimulationExtension> inExtension);

private:
    /**
     * Dispatches any received interaction messages to the appropriate queue.
     */
    void dispatchInteractionMessages();

    /** Used to receive events (through the Network's dispatcher) and to
        send messages. */
    Network& network;

    /** The Lua engine (kept as a pointer to speed up compilation). */
    std::unique_ptr<sol::state> lua;

    /** The world's state. */
    World world;

    /** The tick number that we're currently on. */
    std::atomic<Uint32> currentTick;

    /** The engine's Lua bindings. */
    EngineLuaBindings engineLuaBindings;

    /** If non-nullptr, contains the project's simulation extension functions.
        Allows the project to provide simulation code and have it be called at
        the appropriate time. */
    std::unique_ptr<ISimulationExtension> extension;

    // Note: We receive the generic interaction request messages here and 
    //       dispatch each specific interaction to the appropriate system.
    EventQueue<EntityInteractionRequest> entityInteractionRequestQueue;
    EventQueue<ItemInteractionRequest> itemInteractionRequestQueue;

    /** Holds the subscribed entity interaction queues. */
    std::unordered_map<EntityInteractionType,
                       std::queue<EntityInteractionRequest>*>
        entityInteractionQueueMap;
    /** Holds the subscribed item interaction queues. */
    std::unordered_map<ItemInteractionType, std::queue<ItemInteractionRequest>*>
        itemInteractionQueueMap;

    //-------------------------------------------------------------------------
    // Systems
    //-------------------------------------------------------------------------
    ClientConnectionSystem clientConnectionSystem;
    NceLifetimeSystem nceLifetimeSystem;
    ComponentChangeSystem componentChangeSystem;
    TileUpdateSystem tileUpdateSystem;
    InputSystem inputSystem;
    MovementSystem movementSystem;
    AISystem aiSystem;
    ItemSystem itemSystem;
    InventorySystem inventorySystem;
    ClientAOISystem clientAOISystem;
    MovementSyncSystem movementSyncSystem;
    ComponentSyncSystem componentSyncSystem;
    ChunkStreamingSystem chunkStreamingSystem;
    ScriptDataSystem scriptDataSystem;
    MapSaveSystem mapSaveSystem;
};

} // namespace Server
} // namespace AM
