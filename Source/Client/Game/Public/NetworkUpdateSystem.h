#pragma once

#include "GameDefs.h"
#include <array>

namespace AM
{
namespace Client
{
class Game;
class World;
class Network;

/**
 * This class is in charge of checking for data that needs to be sent, wrapping
 * it appropriately, and passing it to the Network's send queue.
 */
class NetworkUpdateSystem
{
public:
    NetworkUpdateSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * If the player inputs have changed, sends them to the server.
     * Else, sends an empty message as a heartbeat.
     */
    void sendInputState();

private:
    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM
