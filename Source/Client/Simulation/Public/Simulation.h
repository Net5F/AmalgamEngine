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
#include <atomic>

namespace AM
{
class EventDispatcher;

namespace Client
{
class Network;
class SpriteData;

/**
 * Manages the simulation, including world state and system processing.
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

    Uint32 getCurrentTick();

    /**
     * Handles user input events, specifically mouse and momentary events.
     * Note: If the pre-tick handling of these events ever becomes an issue,
     *       we could instead queue them, to be processed during the tick.
     */
    bool handleOSEvent(SDL_Event& event) override;

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

    /**
     * The tick number that we're currently on.
     * Initialized based on the number that the server tells us it's on.
     */
    std::atomic<Uint32> currentTick;

    EventQueue<ConnectionResponse> connectionResponseQueue;

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
