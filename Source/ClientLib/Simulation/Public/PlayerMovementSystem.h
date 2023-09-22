#pragma once

#include "PlayerMovementUpdate.h"
#include "QueuedEvents.h"
#include <SDL_stdinc.h>

namespace AM
{
namespace Client
{
class Simulation;
class World;
class Network;

/**
 * Processes movement update messages for the player's entity and moves the 
 * entity appropriately.
 */
class PlayerMovementSystem
{
public:
    PlayerMovementSystem(Simulation& inSimulation, World& inWorld,
                         Network& inNetwork);

    /**
     * Processes the player entity's movement for this tick.
     *
     * If we received any player entity movement updates from the server,
     * applies them and replays inputs.
     */
    void processMovement();

private:
    /**
     * Processes any waiting player entity updates (sent by the server).
     *
     * @return The tick number of the newest message that we received.
     */
    Uint32 processPlayerUpdates();

    /**
     * Replay any inputs that are from newer ticks than lastUpdateTick.
     */
    void replayInputs(Uint32 lastUpdateTick);

    /**
     * Processes a single tick of player entity movement.
     *
     * @param inputStates The input states to use to move the entity.
     * @post The given velocity, position, and boundingBox now reflect the
     *       entity's new position.
     */
    void movePlayerEntity(Input::StateArr& inputStates);

    /**
     * Calls registry.patch() on each movement-related component to trigger 
     * any on_update callbacks that are connected to them.
     * We don't patch until the end, because we may update the components 
     * multiple times before we're done.
     */
    void triggerUpdateSignals();

    /** Debug printing, asserting. */
    void printMismatchInfo(Uint32 lastUpdateTick);
    void checkReceivedTickValidity(Uint32 updateTick, Uint32 currentTick);
    void checkTickDiffValidity(Uint32 tickDiff);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to get the latest received tick. */
    Network& network;

    EventQueue<PlayerMovementUpdate> playerMovementUpdateQueue;
};

} // namespace Client
} // namespace AM
