#ifndef PLAYERMOVEMENTSYSTEM_H
#define PLAYERMOVEMENTSYSTEM_H

#include "MovementHelpers.h"
#include "GameDefs.h"
#include "InputComponent.h"
#include "Message_generated.h"
#include <array>
#include <vector>
#include <SDL_stdinc.h>

namespace AM
{
namespace Client
{

class Game;
class World;
class Network;

class PlayerMovementSystem
{
public:
    PlayerMovementSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Receives state messages, moves the player, replays inputs.
     */
    void processMovements(Uint64 deltaCount);

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
                      MovementComponent& currentMovement, Uint64 deltaCount);

    Game& game;
    World& world;
    Network& network;

    // Temp
    float lastReceivedX;
    float lastReceivedY;
};

} // namespace Client
} // namespace AM

#endif /* PLAYERMOVEMENTSYSTEM_H */
