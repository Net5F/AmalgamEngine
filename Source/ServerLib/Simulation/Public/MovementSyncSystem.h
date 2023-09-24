#pragma once

#include "entt/entity/registry.hpp"

namespace AM
{
struct Input;
struct Position;
struct Velocity;
struct EntityUpdate;

namespace Server
{
class Simulation;
class World;
class Network;
struct ClientSimData;

/**
 * Sends clients the movement state of any nearby entities that need to be
 * re-synced (including themselves).
 *
 * Reasons for needing to re-sync movement state include:
 *   1. The entity's inputs changed (the user pressed or released a key).
 *   2. We had to drop a movement update request message from the entity
 *      (in such a case, we zero-out their input state so they don't run off
 *      a cliff).
 *   3. The entity was teleported.
 */
class MovementSyncSystem
{
public:
    MovementSyncSystem(Simulation& inSimulation, World& inWorld,
                       Network& inNetwork);

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

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access entity component data. */
    World& world;
    /** Used to send movement update messages. */
    Network& network;

    /** Holds the entities that a particular client needs to be sent updates
        for. */
    std::vector<entt::entity> entitiesToSend;
};

} // namespace Server
} // namespace AM
