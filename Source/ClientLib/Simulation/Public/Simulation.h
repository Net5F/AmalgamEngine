#pragma once

#include "OSEventHandler.h"
#include "World.h"
#include "ServerConnectionSystem.h"
#include "ReplicationTickOffset.h"
#include "ConnectionError.h"
#include "entt/signal/sigh.hpp"
#include <atomic>

namespace AM
{
class EventDispatcher;
struct Item;

namespace Client
{
class Network;
class GraphicData;
class ISimulationExtension;
class ChunkUpdateSystem;
class TileUpdateSystem;
class EntityLifetimeSystem;
class PlayerInputSystem;
class PlayerMovementSystem;
class NpcMovementSystem;
class ItemSystem;
class InventorySystem;
class ComponentUpdateSystem;
class GraphicSystem;
class CameraSystem;

/**
 * Manages the simulation, including world state and system processing.
 *
 * The simulation is built on an ECS architecture:
 *   Entities exist in a registry, owned by the World class.
 *   Components that hold data are attached to each entity.
 *   Systems that act on sets of components are owned and ran by this class.
 */
class Simulation : public OSEventHandler
{
public:
    /** An unreasonable amount of time for the sim tick to be late by. */
    static constexpr double SIM_DELAYED_TIME_S{.001};

    Simulation(EventDispatcher& inUiEventDispatcher, Network& inNetwork,
               GraphicData& inGraphicData);

    ~Simulation();

    /**
     * Returns a reference to the simulation's world state.
     */
    World& getWorld();

    /**
     * Returns our current tick.
     *
     * Our current tick aims to be some amount ahead of the server, so that
     * our messages will arrive before their intended tick is processed.
     *
     * Note: This is used for predicted state (such as player movement).
     */
    Uint32 getCurrentTick();

    /**
     * Returns the tick that we're replicating non-predicted server state at.
     *
     * Our replication tick aims to be some amount behind the server, so that
     * we can smoothly replicate the received server state without network
     * inconsistencies causing choppiness.
     *
     * Note: This is used for non-predicted state (such as NPC movement).
     */
    Uint32 getReplicationTick();

    /**
     * Processes the next sim iteration.
     */
    void tick();

    /**
     * Handles user input events, specifically mouse and momentary events.
     * Note: If the pre-tick handling of these events ever becomes an issue,
     *       we could instead queue them, to be processed during the tick.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<ISimulationExtension> inExtension);

    //-------------------------------------------------------------------------
    // System Signals
    //-------------------------------------------------------------------------
    /** We've established a connection with the server and the simulation has
        started running. */
    entt::sink<entt::sigh<void()>>& getSimulationStartedSink();

    /** Our connection to the server has encountered an error and the
        simulation has stopped running. */
    entt::sink<entt::sigh<void(ConnectionError)>>
        getServerConnectionErrorSink();

    /** We've received the latest definition for an item.
        This may mean that an item was actually updated, or we may have just
        requested the latest data to see if it was updated. */
    entt::sink<entt::sigh<void(const Item&)>>& getItemUpdateSink();

private:
    /**
     * Initializes or re-initializes our simulation systems.
     *
     * Used to put the systems in a consistent state, so they don't need to
     * account for disconnects/reconnects.
     */
    void initializeSystems();

    /** Used to receive events (through the Network's dispatcher) and to
        send messages. */
    Network& network;
    GraphicData& graphicData;

    /** The local world's state. */
    World world;

    /** The tick number that we're currently on.
        Initialized based on the number that the server tells us it's on. */
    std::atomic<Uint32> currentTick;

    /** How far into the past to replicate non-predicted state at. */
    ReplicationTickOffset replicationTickOffset;

    /** If non-nullptr, contains the project's simulation extension functions.
        Allows the project to provide simulation code and have it be called at
        the appropriate time. */
    std::unique_ptr<ISimulationExtension> extension;

    //-------------------------------------------------------------------------
    // Systems
    //-------------------------------------------------------------------------
    // Note: This system is always alive, so it can process connection events.
    ServerConnectionSystem serverConnectionSystem;

    // Note: These are pointers so that we can delete/reconstruct them when we
    //       connect to the server. This gives them a consistent starting state.
    std::unique_ptr<ChunkUpdateSystem> chunkUpdateSystem;
    std::unique_ptr<TileUpdateSystem> tileUpdateSystem;
    std::unique_ptr<EntityLifetimeSystem> entityLifetimeSystem;
    std::unique_ptr<PlayerInputSystem> playerInputSystem;
    std::unique_ptr<PlayerMovementSystem> playerMovementSystem;
    std::unique_ptr<NpcMovementSystem> npcMovementSystem;
    std::unique_ptr<ItemSystem> itemSystem;
    std::unique_ptr<InventorySystem> inventorySystem;
    std::unique_ptr<ComponentUpdateSystem> componentUpdateSystem;
    std::unique_ptr<GraphicSystem> graphicSystem;
    std::unique_ptr<CameraSystem> cameraSystem;
};

} // namespace Client
} // namespace AM
