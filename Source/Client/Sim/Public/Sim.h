#pragma once

#include "NpcMovementSystem.h"
#include "PlayerMovementSystem.h"
#include "World.h"
#include "PlayerInputSystem.h"
#include "NetworkUpdateSystem.h"
#include "CameraSystem.h"
#include "Timer.h"
#include <SDL2pp/Texture.hh>
#include <atomic>

namespace AM
{
namespace Client
{
class Network;

/**
 * Manages the simulation, including world state and system processing.
 */
class Sim
{
public:
    /** An unreasonable amount of time for the sim tick to be late by. */
    static constexpr double SIM_DELAYED_TIME_S = .001;

    Sim(Network& inNetwork, const std::shared_ptr<SDL2pp::Texture>& inSpriteTex);

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
     * Updates accumulatedTime. If greater than the tick timestep, processes
     * the next sim iteration.
     */
    void tick();

    /**
     * Processes all waiting user input events, passing any relevant ones to the
     * playerInputSystem.
     */
    void processUserInputEvents();

    World& getWorld();

    Uint32 getCurrentTick();

    std::atomic<bool> const* getExitRequestedPtr();

private:
    /** How long the sim should wait to receive a connection response from the
        server. */
    static constexpr unsigned int CONNECTION_RESPONSE_WAIT_MS = 1000;

    World world;
    Network& network;

    PlayerInputSystem playerInputSystem;
    NetworkUpdateSystem networkUpdateSystem;
    PlayerMovementSystem playerMovementSystem;
    NpcMovementSystem npcMovementSystem;
    CameraSystem cameraSystem;

    /**
     * The number of the tick that we're currently on.
     * Initialized based on the number that the server tells us it's on.
     */
    std::atomic<Uint32> currentTick;

    // Temporary until a resource manager is created.
    std::shared_ptr<SDL2pp::Texture> spriteTex;

    /**
     * Turn false to signal that the main loop should end.
     * The game processes the inputs, so it gets to be in charge of program
     * lifespan.
     */
    std::atomic<bool> exitRequested;
};

} // namespace Client
} // namespace AM
