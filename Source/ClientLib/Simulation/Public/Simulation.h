#pragma once

#include "OSEventHandler.h"
#include "QueuedEvents.h"
#include "World.h"
#include "ConnectionResponse.h"
#include "ChunkUpdateSystem.h"
#include "TileUpdateSystem.h"
#include "NpcLifetimeSystem.h"
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
    static constexpr double SIM_DELAYED_TIME_S = .001;

    Simulation(EventDispatcher& inUiEventDispatcher, Network& inNetwork,
               SpriteData& inSpriteData);

    /**
     * Requests to connect to the game server, waits for an assigned EntityID,
     * and constructs the player.
     */
    void connect();

    /**
     * Fills in player data as if we connected to the server.
     */
    void fakeConnection();

    /**
     * Processes the next sim iteration.
     */
    void tick();

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
    /** How long the sim should wait for the server to send a connection
        response, in microseconds. */
    static constexpr int CONNECTION_RESPONSE_WAIT_US = 1 * 1000 * 1000;

    /** Used to receive events (through the Network's dispatcher) and to
        send messages. */
    Network& network;

    /** Temporarily used for loading the player's sprite texture.
        When that logic gets moved, this member can be removed. */
    SpriteData& spriteData;

    World world;

    /** The tick number that we're currently on.
        Initialized based on the number that the server tells us it's on. */
    std::atomic<Uint32> currentTick;

    /** How far into the past to replicate non-predicted state at. */
    ReplicationTickOffset replicationTickOffset;

    EventQueue<ConnectionResponse> connectionResponseQueue;

    /** If non-nullptr, contains the project's simulation extension functions.
        Allows the project to provide simulation code and have it be called at
        the appropriate time. */
    std::unique_ptr<ISimulationExtension> extension;

    //-------------------------------------------------------------------------
    // Systems
    //-------------------------------------------------------------------------
    ChunkUpdateSystem chunkUpdateSystem;
    TileUpdateSystem tileUpdateSystem;
    NpcLifetimeSystem npcLifetimeSystem;
    PlayerInputSystem playerInputSystem;
    PlayerMovementSystem playerMovementSystem;
    NpcMovementSystem npcMovementSystem;
    CameraSystem cameraSystem;
};

} // namespace Client
} // namespace AM
