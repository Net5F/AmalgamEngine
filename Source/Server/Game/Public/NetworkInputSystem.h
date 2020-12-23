#pragma once

#include "entt/entity/registry.hpp"

namespace AM
{
namespace Server
{
class Game;
class World;
class Network;

/**
 * This system is in charge of receiving input messages from clients and
 * applying them to the client's entity.
 */
class NetworkInputSystem
{
public:
    NetworkInputSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Processes incoming EntityUpdate messages.
     */
    void processInputMessages();

private:
    /**
     * Processes message drop events, which occur when the server received a
     * client's input message late and had to drop it.
     *
     * We default the client's inputs (so they don't run off a cliff) and set
     * a flag so the NetworkUpdateSystem knows that a drop occurred.
     */
    void processMessageDropEvents();

    /**
     * Defaults the entity's inputs and marks them dirty if their inputs
     * changed.
     */
    void handleDropForEntity(entt::entity entityID);

    Game& game;
    World& world;
    Network& network;
};

} // namespace Server
} // namespace AM
