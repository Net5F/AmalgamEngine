#pragma once

#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include <vector>

namespace AM
{
namespace Server
{
/**
 * Acts as a tag to say "this entity belongs to a client". Entities without
 * this component are dummies or are AI-controlled.
 *
 * Also holds all client-related sim data.
 *
 * Note: If this ever gets too big, we can change it to "IsClient" or
 *       "ClientNetID" and split the sim data out into other components.
 */
struct ClientSimData {
public:
    /** The network ID associated with this client.
        We track this here so the sim knows where to send messages related to
        the entity.
        We also use this to remove the entity from the sim when the Client
        disconnects.*/
    NetworkID netID{0};

    /** Tracks the entities that are in range of this client's entity. */
    std::vector<entt::entity> entitiesInAOI{};

    /** Tracks the entities that have just entered this client's AOI on this
        tick.
        Only valid after ClientAOISystem has ran. */
    std::vector<entt::entity> entitiesThatEnteredAOI{};
};

} // namespace Server
} // namespace AM
