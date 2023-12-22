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

    // Note: Since the <Entity|Item>InteractionRequest messages are consumed by 
    //       different systems depending on their type, we receive and queue 
    //       the messages here. Systems can use these functions to pop them.
    struct EntityInteractionData
    {
        /** The ID of the entity performing the interaction. */
        entt::entity clientEntity{entt::null};
        /** The ID of the entity that the interaction is being performed on. */
        entt::entity targetEntity{entt::null};
        /** The network ID of the client performing the interaction. */
        NetworkID clientID{0};
    };
    /**
     * Pops the next entity interaction request of the given type off the queue, 
     * checks it for validity, and returns its data.
     *
     * Entity interactions occur when the user left-clicks an entity, or right-
     * clicks and selects an interaction from the menu.
     *
     * @param interactionType The type of interaction request to return.
     * @param[out] data If return==true, holds the interaction data.
     * @return true if a valid request was waiting, else false.
     */
    bool popEntityInteractionRequest(EntityInteractionType interactionType,
                                     EntityInteractionData& data);

    struct ItemInteractionData
    {
        /** The ID of the entity performing the interaction. */
        entt::entity clientEntity{entt::null};
        /** The inventory slot of the item that the interaction is being 
            performed on. */
        Uint8 slotIndex{0};
        /** The item that the interaction is being performed on. */
        const Item* item{nullptr};
        /** The network ID of the client performing the interaction. */
        NetworkID clientID{0};
    };
    /**
     * Pops the next item interaction request of the given type off the queue, 
     * checks it for validity, and returns its data.
     *
     * Item interactions occur when the user left-clicks an item, or right-
     * clicks and selects an interaction from the menu.
     *
     * @param interactionType The type of interaction request to return.
     * @param[out] data If return==true, holds the interaction data.
     * @return true if a valid request was waiting, else false.
     */
    bool popItemInteractionRequest(ItemInteractionType interactionType,
                                   ItemInteractionData& data);

    /**
     * Returns a reference to the simulation's world state.
     */
    World& getWorld();

    /**
     * Returns a reference to the simulation's Lua bindings for entity init 
     * processing.
     */
    sol::state& getEntityInitLua();

    /**
     * Returns a reference to the simulation's Lua bindings for entity item  
     * handler processing.
     */
    sol::state& getEntityItemHandlerLua();

    /**
     * Returns a reference to the simulation's Lua bindings for item init 
     * processing.
     */
    sol::state& getItemInitLua();

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
     * Sorts any received interaction messages to the appropriate queue.
     */
    void sortInteractionMessages();

    /** Used to receive events (through the Network's dispatcher) and to
        send messages. */
    Network& network;

    /** Lua environment for entity init script processing.
        Kept as a pointer to speed up compilation. */
    std::unique_ptr<sol::state> entityInitLua;

    /** Lua environment for entity item handler script processing. */
    std::unique_ptr<sol::state> entityItemHandlerLua;

    /** Lua environment for item init script processing. */
    std::unique_ptr<sol::state> itemInitLua;

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

    /** Holds the received entity interaction requests of each type. */
    std::unordered_map<EntityInteractionType,
                       std::queue<EntityInteractionRequest>>
        entityInteractionQueueMap;
    /** Holds the received item interaction requests of each type. */
    std::unordered_map<ItemInteractionType, std::queue<ItemInteractionRequest>>
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
