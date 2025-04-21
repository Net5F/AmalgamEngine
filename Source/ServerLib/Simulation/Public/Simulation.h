#pragma once

#include "World.h"
#include "EngineLuaBindings.h"
#include "ClientConnectionSystem.h"
#include "NceLifetimeSystem.h"
#include "ComponentChangeSystem.h"
#include "TileUpdateSystem.h"
#include "InputSystem.h"
#include "MovementSystem.h"
#include "AISystem.h"
#include "CastSystem.h"
#include "ItemSystem.h"
#include "InventorySystem.h"
#include "DialogueSystem.h"
#include "ClientAOISystem.h"
#include "MovementSyncSystem.h"
#include "ComponentSyncSystem.h"
#include "ChunkStreamingSystem.h"
#include "ScriptDataSystem.h"
#include "SaveSystem.h"
#include <SDL_stdinc.h>
#include <atomic>
#include <memory>

namespace AM
{
class CastableData;

namespace Server
{
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

    Simulation(Network& inNetwork, GraphicData& inGraphicData,
               ItemData& inItemData, CastableData& inCastableData);

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
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<ISimulationExtension> inExtension);

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

    /** If non-nullptr, contains the project's simulation extension functions.
        Allows the project to provide simulation code and have it be called at
        the appropriate time. */
    std::unique_ptr<ISimulationExtension> extension;

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
    CastSystem castSystem;
    ItemSystem itemSystem;
    InventorySystem inventorySystem;
    DialogueSystem dialogueSystem;
    ClientAOISystem clientAOISystem;
    MovementSyncSystem movementSyncSystem;
    ComponentSyncSystem componentSyncSystem;
    ChunkStreamingSystem chunkStreamingSystem;
    ScriptDataSystem scriptDataSystem;
    SaveSystem saveSystem;
};

} // namespace Server
} // namespace AM
