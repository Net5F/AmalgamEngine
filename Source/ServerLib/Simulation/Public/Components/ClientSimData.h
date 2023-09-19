#pragma once

#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include <vector>

namespace AM
{
namespace Server
{
/**
 * Holds all client-related sim data.
 *
 * If you need to tell if an entity is a client entity or not, favor checking 
 * for IsClientEntity over this. IsClientEntity gets replicated to the clients,
 * so it's more expected to see.
 */
struct ClientSimData {
    /** The network ID associated with this client.
        We track this here so the sim knows where to send messages related to
        the entity.
        We also use this to remove the entity from the sim when the Client
        disconnects.*/
    NetworkID netID{0};

    /** Tracks the entities that are in range of this client's entity. */
    std::vector<entt::entity> entitiesInAOI{};
};

} // namespace Server
} // namespace AM
