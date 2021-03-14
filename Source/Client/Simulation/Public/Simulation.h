#pragma once

#include "EventHandler.h"
#include "ResourceManager.h"
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
class Simulation : public EventHandler
{
public:
    /** An unreasonable amount of time for the sim tick to be late by. */
    static constexpr double SIM_DELAYED_TIME_S = .001;

    Simulation(Network& inNetwork, ResourceManager& inResourceManager);

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

    World& getWorld();

    Uint32 getCurrentTick();

    /**
     * Handles user input events, specifically mouse and momentary events.
     * Note: If the pre-tick handling of these events ever becomes an issue,
     *       we could instead queue them, to be processed during the tick.
     */
    bool handleEvent(SDL_Event& event) override;

private:
    /** How long the sim should wait to receive a connection response from the
        server. */
    static constexpr unsigned int CONNECTION_RESPONSE_WAIT_MS = 1000;

    World world;
    Network& network;

    ResourceManager& resourceManager;

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
};

} // namespace Client
} // namespace AM
