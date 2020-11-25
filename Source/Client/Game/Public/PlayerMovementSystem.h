#pragma once

#include "MovementHelpers.h"
#include "GameDefs.h"
#include "InputComponent.h"
#include <array>
#include <vector>

namespace AM
{
namespace Client
{
class Game;
class World;
class Network;

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
    Uint32 processPlayerUpdates();

    /**
     * Replay any inputs that are from newer ticks than the latestReceivedTick.
     */
    void replayInputs(Uint32 latestReceivedTick);

    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM
