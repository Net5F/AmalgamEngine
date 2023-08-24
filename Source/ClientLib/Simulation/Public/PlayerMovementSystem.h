#pragma once

#include <SDL_stdinc.h>
#include "QueuedEvents.h"
#include "MovementUpdate.h"

namespace AM
{
struct Position;
struct PreviousPosition;
struct Velocity;
struct Input;
struct Rotation;
struct Collision;

namespace Client
{
class Simulation;
class World;
class Network;

struct InputHistory;

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
    Uint32 processPlayerUpdates(Position& position,
                                PreviousPosition& previousPosition,
                                Velocity& velocity, Input& input,
                                InputHistory& inputHistory, Rotation& rotation,
                                Collision& collision);

    /**
     * Replay any inputs that are from newer ticks than the latestReceivedTick.
     */
    void replayInputs(Uint32 latestReceivedTick, Position& position,
                      Velocity& velocity, Rotation& rotation,
                      InputHistory& inputHistory, Collision& collision);

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

    /**
     * Processes a single tick of player entity movement.
     *
     * @post The given velocity, position, and boundingBox now reflect the
     *       entity's new position.
     */
    void movePlayerEntity(Input::StateArr& inputStates, Velocity& velocity,
                          Position& position, Rotation& rotation,
                          Collision& collision);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;

    EventQueue<std::shared_ptr<const MovementUpdate>> playerUpdateQueue;
};

} // namespace Client
} // namespace AM
