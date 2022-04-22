#pragma once

#include "entt/fwd.hpp"
#include <vector>

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;
struct ClientSimData;

/**
 * Maintains each client entity's list of peers that are within their area of
 * interest.
 *
 * When a peer enters a client entity's AOI, this system will update the lists
 * appropriately and send an EntityInit message to the client.
 *
 * When a peer leaves a client entity's AOI, this system will update the lists
 * appropriately and send an EntityDelete message to the client.
 *
 * Note: The AOI lists also must be updated when an entity disconnects. Since
 *       it's easiest to do this while the entity is still alive, and
 *       ClientConnectionSystem maintains the lifetime of client entities, it's
 *       handled there.
 */
class ClientAOISystem
{
public:
    ClientAOISystem(Simulation& inSim, World& inWorld, Network& inNetwork);

    /**
     * Updates the peersInAOI list in any client entities that have recently
     * moved.
     *
     * If entities have entered/left a list, updates peer lists and sends
     * messages appropriately.
     */
    void updateAOILists();

private:
    /**
     * Processes the entities that left entityThatMoved's AOI.
     * Updates lists and sends EntityDelete messages appropriately.
     */
    void processEntitiesThatLeft(entt::entity entityThatMoved,
                                 ClientSimData& clientThatMoved);

    /**
     * Processes the entities that entered entityThatMoved's AOI.
     * Updates lists and sends EntityInit messages appropriately.
     */
    void processEntitiesThatEntered(entt::entity entityThatMoved,
                                    ClientSimData& clientThatMoved);

    /** Used to get the current tick number. */
    Simulation& sim;
    /** Used to access components. */
    World& world;
    /** Used for sending messages. */
    Network& network;

    /** Holds entities that left the AOI. Used during updateAOILists(). */
    std::vector<entt::entity> entitiesThatLeft;
};

} // End namespace Server
} // End namespace AM
