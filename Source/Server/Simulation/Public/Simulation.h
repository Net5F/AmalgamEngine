#pragma once

#include "World.h"
#include "ClientConnectionSystem.h"
#include "TileUpdateSystem.h"
#include "InputUpdateSystem.h"
#include "MovementSystem.h"
#include "ClientUpdateSystem.h"
#include "ChunkStreamingSystem.h"
#include <SDL2/SDL_stdinc.h>
#include <atomic>

namespace AM
{
namespace Server
{
class Network;
class SpriteData;

/**
 * Manages the simulation, including world state and system processing.
 */
class Simulation
{
public:
    /** An unreasonable amount of time for the sim tick to be late by. */
    static constexpr double SIM_DELAYED_TIME_S = .001;

    Simulation(EventDispatcher& inNetworkEventDispatcher, Network& inNetwork, SpriteData& inSpriteData);

    /**
     * Updates accumulatedTime. If greater than the tick timestep, processes
     * the next sim iteration.
     */
    void tick();

    Uint32 getCurrentTick();

private:
    /** Used to receive events (through the Network's dispatcher) and to
        send messages. */
    Network& network;

    World world;

    ClientConnectionSystem clientConnectionSystem;
    TileUpdateSystem tileUpdateSystem;
    InputUpdateSystem inputUpdateSystem;
    MovementSystem movementSystem;
    ClientUpdateSystem clientUpdateSystem;
    ChunkStreamingSystem chunkStreamingSystem;

    /**
     * The number of the tick that we're currently on.
     */
    std::atomic<Uint32> currentTick;
};

} // namespace Server
} // namespace AM
