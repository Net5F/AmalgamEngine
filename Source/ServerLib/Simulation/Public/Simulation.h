#pragma once

#include "World.h"
#include "ClientConnectionSystem.h"
#include "TileUpdateSystem.h"
#include "ClientAOISystem.h"
#include "NceLifetimeSystem.h"
#include "InputSystem.h"
#include "MovementSystem.h"
#include "MovementSyncSystem.h"
#include "ChunkStreamingSystem.h"
#include "MapSaveSystem.h"
#include <SDL_stdinc.h>
#include <atomic>

namespace AM
{
namespace Server
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

    World& getWorld();

    Uint32 getCurrentTick();

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<ISimulationExtension> inExtension);

private:
    /** Used to receive events (through the Network's dispatcher) and to
        send messages. */
    Network& network;

    World world;

    /** The tick number that we're currently on. */
    std::atomic<Uint32> currentTick;

    /** If non-nullptr, contains the project's simulation extension functions.
        Allows the project to provide simulation code and have it be called at
        the appropriate time. */
    std::unique_ptr<ISimulationExtension> extension;

    //-------------------------------------------------------------------------
    // Systems
    //-------------------------------------------------------------------------
    ClientConnectionSystem clientConnectionSystem;
    TileUpdateSystem tileUpdateSystem;
    ClientAOISystem clientAOISystem;
    NceLifetimeSystem nceLifetimeSystem;
    InputSystem inputSystem;
    MovementSystem movementSystem;
    MovementSyncSystem movementSyncSystem;
    ChunkStreamingSystem chunkStreamingSystem;
    MapSaveSystem mapSaveSystem;
};

} // namespace Server
} // namespace AM
