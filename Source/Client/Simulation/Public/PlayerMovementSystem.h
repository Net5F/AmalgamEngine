#pragma once

#include <SDL2/SDL_stdinc.h>
#include "QueuedEvents.h"
#include "EntityUpdate.h"

namespace AM
{
class Position;
class PreviousPosition;
class Velocity;
class Input;

namespace Client
{
class Simulation;
class World;
class Network;

class InputHistory;

/**
 * Processes player entity update messages and moves the entity appropriately.
 */
class PlayerMovementSystem
{
public:
    PlayerMovementSystem(Simulation& inSim, World& inWorld,
                         EventDispatcher& inNetworkEventDispatcher);

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
    Uint32 processPlayerUpdates(Position& position,
                                PreviousPosition& previousPosition,
                                Velocity& velocity, Input& input,
                                InputHistory& inputHistory);

    /**
     * Replay any inputs that are from newer ticks than the latestReceivedTick.
     */
    void replayInputs(Uint32 latestReceivedTick, Position& position,
                      Velocity& velocity, InputHistory& inputHistory);

    /**
     * If receivedTick > currentTick, logs an error.
     * Used to move a large error print out of a long function.
     */
    void checkReceivedTickValidity(Uint32 receivedTick, Uint32 currentTick);

    /**
     * If tickDiff is larger than the number of elements we have in the
     * player's input history, logs an error.
     * Used to move a large error print out of a long function.
     */
    void checkTickDiffValidity(Uint32 tickDiff);

    /** Used to get the current tick. */
    Simulation& sim;
    /** Used to access components. */
    World& world;

    EventQueue<std::shared_ptr<const EntityUpdate>> playerUpdateQueue;
};

} // namespace Client
} // namespace AM
