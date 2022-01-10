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
 * Sends each client the new movement state of any nearby entities that moved.
 */
class MovementUpdateSystem
{
public:
    MovementUpdateSystem(Simulation& inSim, World& inWorld, Network& inNetwork);

    /**
     * Updates all connected clients with relevant entity movement state.
     */
    void sendMovementUpdates();

private:
    /**
     * Determines which entity's data needs to be sent to the given client and
     * adds them to entitiesToSend.
     *
     * Will add any entities that have just entered the client's AOI, and any
     * entities already within the client's AOI that have changed input state.
     */
    void collectEntitiesToSend(ClientSimData& client,
                               entt::entity clientEntity);

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
