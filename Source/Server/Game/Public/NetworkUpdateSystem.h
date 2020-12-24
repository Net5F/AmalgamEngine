#pragma once

#include "GameDefs.h"
#include "EntityState.h"
#include "entt/entity/registry.hpp"
#include <vector>

namespace AM
{
namespace Server
{
class Game;
class World;
class Network;

/**
 * This system is in charge of checking for data that needs to be sent to
 * clients, wrapping it appropriately, and passing it to the Network's send
 * queue.
 */
class NetworkUpdateSystem
{
public:
    NetworkUpdateSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Updates all connected clients with relevant world state.
     */
    void sendClientUpdates();

private:
    /**
     * Fills the given vector with the entities that must be sent to the given
     * entityID on this tick.
     */
    void constructAndSendUpdate(entt::entity entity,
                                std::vector<entt::entity>& entitiesToSend);

    /**
     * Serializes the given entity's relevant world
     * @param entity  The entity to serialize.
     * @return An offset where the data was stored in the builder.
     */
    void fillEntityData(entt::entity entity,
                        std::vector<EntityState>& entityStates);

    Game& game;
    World& world;
    Network& network;
};

} // namespace Server
} // namespace AM
