#ifndef PLAYERMOVEMENTSYSTEM_H
#define PLAYERMOVEMENTSYSTEM_H

#include "MovementHelpers.h"
#include "GameDefs.h"
#include "InputComponent.h"
#include "Message_generated.h"
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
    Uint32 processReceivedUpdates(EntityID playerID, PositionComponent& currentPosition,
                                  MovementComponent& currentMovement);

    /**
     * Replay any inputs that are from newer ticks than the latestReceivedTick.
     */
    void replayInputs(Uint32 latestReceivedTick, PositionComponent& currentPosition,
                      MovementComponent& currentMovement);

    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM

#endif /* PLAYERMOVEMENTSYSTEM_H */
