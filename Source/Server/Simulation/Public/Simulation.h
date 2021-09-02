#pragma once

#include "World.h"
#include "NetworkConnectionSystem.h"
#include "NetworkInputSystem.h"
#include "MovementSystem.h"
#include "NetworkUpdateSystem.h"
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

    Simulation(Network& inNetwork, SpriteData& inSpriteData);

    /**
     * Updates accumulatedTime. If greater than the tick timestep, processes
     * the next sim iteration.
     */
    void tick();

    Uint32 getCurrentTick();

private:
    World world;
    Network& network;

    NetworkConnectionSystem networkConnectionSystem;
    NetworkInputSystem networkInputSystem;
    MovementSystem movementSystem;
    NetworkUpdateSystem networkUpdateSystem;

    /**
     * The number of the tick that we're currently on.
     */
    std::atomic<Uint32> currentTick;
};

} // namespace Server
} // namespace AM
