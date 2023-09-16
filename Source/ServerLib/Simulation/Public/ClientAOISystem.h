#pragma once

#include "BinaryBuffer.h"
#include "entt/fwd.hpp"
#include <vector>
#include <unordered_map>

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
 * Note: We split "init" logic (handled here) from "update" logic (handled in 
 *       systems). This is because updates sometimes require more context, e.g.
 *       movement updates are triggered when an Input is updated, but must send 
 *       Input, Position, Velocity, and Rotation components.
 *       Also, inits are built by testing each client's AOI, whereas updates are
 *       built by testing the updated entity's AOI.
 */
class ClientAOISystem
{
public:
    ClientAOISystem(Simulation& inSimulation, World& inWorld,
                    Network& inNetwork);

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
     * Sends an EntityDelete message to the given client for each entity that
     * left its AOI.
     */
    void processEntitiesThatLeft(ClientSimData& client);

    /**
     * Sends an EntityInit message to the given client for each entity that
     * entered its AOI.
     */
    void processEntitiesThatEntered(ClientSimData& client);

    /** Used to get the current tick number. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used for sending messages. */
    Network& network;

    /** Holds entities that left the AOI. Used during updateAOILists(). */
    std::vector<entt::entity> entitiesThatLeft;

    /** Holds entities that entered the AOI. Used during updateAOILists(). */
    std::vector<entt::entity> entitiesThatEntered;

    /** Maps entityID -> a serialized EntityInit message containing that 
        entity's data. We use this map to save the cost of building the same 
        message multiple times (once for each client that got in range).
        We clear this out each tick, since it would be invalidated by any 
        system changing any of the included components. */
    std::unordered_map<entt::entity, BinaryBufferSharedPtr> entityInitMap;
};

} // End namespace Server
} // End namespace AM
