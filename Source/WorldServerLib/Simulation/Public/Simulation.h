#pragma once

#include "World.h"
#include "EngineLuaBindings.h"
#include "ClientConnectionSystem.h"
#include "NonPlayerEntityLifetimeSystem.h"
#include "ComponentChangeSystem.h"
#include "TileUpdateSystem.h"
#include "InputSystem.h"
#include "AISystem.h"
#include "MovementSystem.h"
#include "CastSystem.h"
#include "ItemSystem.h"
#include "InventorySystem.h"
#include "DialogueSystem.h"
#include "ClientAOISystem.h"
#include "MovementSyncSystem.h"
#include "ComponentSyncSystem.h"
#include "ChunkStreamingSystem.h"
#include "ScriptDataSystem.h"
#include "ChatPositionSyncSystem.h"
#include "SaveSystem.h"
#include <SDL3/SDL_stdinc.h>
#include <atomic>
#include <chrono>
#include <memory>

namespace AM
{
class CastableData;

namespace WorldServer
{
struct SimulationContext;
class Network;
class GraphicData;
class ItemData;
struct EntityInitLua;
struct EntityItemHandlerLua;
struct ItemInitLua;
struct DialogueLua;
struct DialogueChoiceConditionLua;
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

    /** How long shutdown() will wait for AccountData saves to be confirmed
        by the account server. Long enough for a couple of retries. */
    static constexpr std::chrono::seconds SHUTDOWN_ACCOUNT_SAVE_TIMEOUT{15};

    Simulation(const SimulationContext& inSimContext);

    ~Simulation();

    /**
     * Returns a reference to the simulation's world state.
     */
    World& getWorld();

    /**
     * Returns a reference to the simulation's Lua bindings.
     */
    EntityInitLua& getEntityInitLua();
    EntityItemHandlerLua& getEntityItemHandlerLua();
    ItemInitLua& getItemInitLua();
    DialogueLua& getDialogueLua();
    DialogueChoiceConditionLua& getDialogueChoiceConditionLua();

    /**
     * Returns the simulation's current tick number.
     */
    Uint32 getCurrentTick() const;

    /**
     * Updates accumulatedTime. If greater than the tick timestep, processes
     * the next sim iteration.
     */
    void tick();

    /**
     * Saves all world state, backs it up to the file database, and waits for
     * player entity AccountData saves to finish (up to
     * SHUTDOWN_ACCOUNT_SAVE_TIMEOUT).
     *
     * Call this once when the server is shutting down, after the last tick.
     */
    void shutdown();

    /**
     * See extension member comment.
     */
    void setExtension(ISimulationExtension* inExtension);

private:
    /** Used to receive events (through the Network's dispatcher) and to
        send messages. */
    Network& network;

    /** Lua environment for entity init script processing.
        Kept as a pointer to speed up compilation. */
    std::unique_ptr<EntityInitLua> entityInitLua;

    /** Lua environment for entity item handler script processing. */
    std::unique_ptr<EntityItemHandlerLua> entityItemHandlerLua;

    /** Lua environment for item init script processing. */
    std::unique_ptr<ItemInitLua> itemInitLua;

    /** Lua environment for dialogue topic and choice action script
        processing. */
    std::unique_ptr<DialogueLua> dialogueLua;

    /** Lua environment for dialogue choice condition script processing. */
    std::unique_ptr<DialogueChoiceConditionLua> dialogueChoiceConditionLua;

    /** The world's state. */
    World world;

    /** The tick number that we're currently on. */
    std::atomic<Uint32> currentTick;

    /** The engine's Lua bindings. */
    EngineLuaBindings engineLuaBindings;

    /** Contains the project's simulation extension functions.
        Allows the project to provide simulation code and have it be called at
        the appropriate time. */
    ISimulationExtension* extension;

    //-------------------------------------------------------------------------
    // Systems
    //-------------------------------------------------------------------------
    ClientConnectionSystem clientConnectionSystem;
    NonPlayerEntityLifetimeSystem nonPlayerEntityLifetimeSystem;
    ComponentChangeSystem componentChangeSystem;
    TileUpdateSystem tileUpdateSystem;
    InputSystem inputSystem;
    AISystem aiSystem;
    MovementSystem movementSystem;
    CastSystem castSystem;
    ItemSystem itemSystem;
    InventorySystem inventorySystem;
    DialogueSystem dialogueSystem;
    ClientAOISystem clientAOISystem;
    MovementSyncSystem movementSyncSystem;
    ComponentSyncSystem componentSyncSystem;
    ChunkStreamingSystem chunkStreamingSystem;
    ScriptDataSystem scriptDataSystem;
    ChatPositionSyncSystem chatPositionSyncSystem;
    SaveSystem saveSystem;
};

} // namespace WorldServer
} // namespace AM
