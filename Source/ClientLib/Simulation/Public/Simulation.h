#pragma once

#include "OSEventHandler.h"
#include "World.h"
#include "ServerConnectionSystem.h"
#include "ChunkUpdateSystem.h"
#include "TileUpdateSystem.h"
#include "SpriteUpdateSystem.h"
#include "EntityLifetimeSystem.h"
#include "PlayerInputSystem.h"
#include "PlayerMovementSystem.h"
#include "NpcMovementSystem.h"
#include "CameraSystem.h"
#include "Timer.h"
#include "ReplicationTickOffset.h"
#include <atomic>

namespace AM
{
class EventDispatcher;

namespace Client
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
class Simulation : public OSEventHandler
{
public:
    /** An unreasonable amount of time for the sim tick to be late by. */
    static constexpr double SIM_DELAYED_TIME_S{.001};

    Simulation(EventDispatcher& inUiEventDispatcher, Network& inNetwork,
               SpriteData& inSpriteData);

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

private:
    /** Used to receive events (through the Network's dispatcher) and to
        send messages. */
    Network& network;

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
    ServerConnectionSystem serverConnectionSystem;
    ChunkUpdateSystem chunkUpdateSystem;
    TileUpdateSystem tileUpdateSystem;
    SpriteUpdateSystem spriteUpdateSystem;
    EntityLifetimeSystem entityLifetimeSystem;
    PlayerInputSystem playerInputSystem;
    PlayerMovementSystem playerMovementSystem;
    NpcMovementSystem npcMovementSystem;
    CameraSystem cameraSystem;
};

} // namespace Client
} // namespace AM
