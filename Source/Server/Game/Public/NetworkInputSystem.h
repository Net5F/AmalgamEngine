#pragma once

#include "InputComponent.h"

namespace AM
{
namespace Server
{

class Game;
class World;
class Network;

class NetworkInputSystem
{
public:
    NetworkInputSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Processes incoming EntityUpdate messages.
     */
    void processInputMessages();

private:
    Game& game;
    World& world;
    Network& network;
};

} // namespace Server
} // namespace AM
