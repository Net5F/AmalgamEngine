#pragma once

#include "entt/entity/registry.hpp"

namespace AM
{
class Input;
class Position;
class Movement;
class EntityUpdate;

namespace Server
{
class Simulation;
class World;
class Network;
class ClientSimData;

/**
 * This system is in charge of checking for entity state that needs to be sent
 * to clients, wrapping it appropriately, and passing it to the Network's send
 * queue.
 */
class ClientUpdateSystem
{
public:
    ClientUpdateSystem(Simulation& inSim, World& inWorld, Network& inNetwork);

    /**
     * Updates all connected clients with relevant entity state.
     */
    void sendClientUpdates();

private:
    /**
     * Holds references to relevant entity state.
     * Note: These references are potentially invalidated whenever the
     *       component pool changes. We just use them locally here.
     */
    struct EntityStateRefs {
        entt::entity entity;
        Input& input;
        Position& position;
        Movement& movement;
    };

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
