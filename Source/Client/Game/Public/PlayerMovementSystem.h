#ifndef PLAYERMOVEMENTSYSTEM_H
#define PLAYERMOVEMENTSYSTEM_H

#include "MovementHelpers.h"
#include "SharedDefs.h"
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

class PlayerMovementSystem
{
public:
    PlayerMovementSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Receives state messages, moves the player, replays inputs.
     */
    void processMovements(float deltaSeconds);

private:
    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM

#endif /* PLAYERMOVEMENTSYSTEM_H */
