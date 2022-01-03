#pragma once

#include "entt/entity/registry.hpp"

namespace AM
{
class Input;
class Position;
class Velocity;
class EntityUpdate;

namespace Server
{
class Simulation;
class World;
class Network;
class ClientSimData;

/**
 * Checks for entity movement state that needs to be sent to clients, wraps it
 * appropriately, and passes it to the Network's send queue.
 */
class ClientUpdateSystem
{
public:
    ClientUpdateSystem(Simulation& inSim, World& inWorld, Network& inNetwork);

    /**
     * Updates all connected clients with relevant entity movement state.
     */
    void sendClientUpdates();

private:
    /**
     * Determines which entity's data needs to be sent to the given client and
     * adds them to entitiesToSend.
     *
     * Will add any entities that have just entered the client's AOI, and any
     * entities already within the client's AOI that have changed input state.
     */
    void collectEntitiesToSend(ClientSimData& client, entt::entity clientEntity);

    /**
     * Adds the movement state of all entities in entitiesToSend to an
     * EntityUpdate message and sends it to the given client.
     */
    void sendEntityUpdate(ClientSimData& client);

    Simulation& sim;
    World& world;
    Network& network;

    /** Holds the entities that a particular client needs to be sent updates
        for.
        Used during updateClient(). */
    std::vector<entt::entity> entitiesToSend;
};

} // namespace Server
} // namespace AM
