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
struct BoundingBox;

namespace Client
{
class Simulation;
class World;
class Network;
struct Sprite;

struct InputHistory;

/**
 * Processes player entity update messages and moves the entity appropriately.
 */
class PlayerMovementSystem
{
public:
    PlayerMovementSystem(Simulation& inSimulation, World& inWorld,
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
                                InputHistory& inputHistory, Sprite& sprite,
                                BoundingBox& boundingBox);

    /**
     * Replay any inputs that are from newer ticks than the latestReceivedTick.
     */
    void replayInputs(Uint32 latestReceivedTick, Position& position,
                      Velocity& velocity, InputHistory& inputHistory,
                      Sprite& sprite, BoundingBox& boundingBox);

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
                          Position& position, Sprite& sprite,
                          BoundingBox& boundingBox);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;

    EventQueue<std::shared_ptr<const MovementUpdate>> playerUpdateQueue;
};

} // namespace Client
} // namespace AM
