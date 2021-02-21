#pragma once

#include "SimDefs.h"
#include "entt/entity/registry.hpp"
#include <vector>

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
 * This system is in charge of checking for data that needs to be sent to
 * clients, wrapping it appropriately, and passing it to the Network's send
 * queue.
 */
class NetworkUpdateSystem
{
public:
    NetworkUpdateSystem(Simulation& inSim, World& inWorld, Network& inNetwork);

    /**
     * Updates all connected clients with relevant world state.
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
     * Fills the given vector with the entities that must be sent to the given
     * entityID on this tick.
     */
    void sendUpdate(ClientSimData& client, EntityUpdate& entityUpdate);

    Simulation& sim;
    World& world;
    Network& network;
};

} // namespace Server
} // namespace AM
