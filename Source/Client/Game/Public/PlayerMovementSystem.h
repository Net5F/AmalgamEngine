#pragma once

#include "MovementHelpers.h"
#include "GameDefs.h"
#include <array>
#include <vector>

namespace AM
{
class Position;
class PreviousPosition;
class Movement;
class Input;

namespace Client
{
class Game;
class World;
class Network;

class PlayerState;

/**
 * Processes player entity update messages and moves the entity appropriately.
 */
class PlayerMovementSystem
{
public:
    PlayerMovementSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Moves the player entity 1 sim tick into the future.
     * Receives state messages, moves the player, replays inputs.
     */
    void processMovements();

private:
    /**
     * Receives any player entity updates from the server.
     * @return The tick number of the newest message that we received.
     */
    Uint32 processPlayerUpdates(Position& currentPosition,
                                PreviousPosition& previousPosition,
                                Movement& currentMovement, Input& currentInput,
                                PlayerState& playerState);

    /**
     * Replay any inputs that are from newer ticks than the latestReceivedTick.
     */
    void replayInputs(Uint32 latestReceivedTick, Position& currentPosition,
                      Movement& currentMovement, PlayerState& playerState);

    /** If receivedTick > currentTick, logs an error. */
    void checkReceivedTickValidity(Uint32 receivedTick, Uint32 currentTick);

    /** If tickDiff is larger than the number of elements we have in the
        player's input history, logs an error. */
    void checkTickDiffValidity(Uint32 tickDiff);

    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM
