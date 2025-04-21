#pragma once

#include "NetworkID.h"
#include "Vector3.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <functional>

namespace AM
{
struct Castable;
struct Item;

/**
 * All of the information necessary to perform a cast.
 */
struct CastInfo {
    /** The Castable that's being cast. Should always be non-nullptr. */
    const Castable* castable{nullptr};

    /** The ID of the entity performing the cast.
        May or may not be a client entity. */
    entt::entity casterEntity{entt::null};

    /** If this is an ItemInteraction cast, this is the item that's being 
        used. 
        Note: This field isn't filled when using this struct in Client code. 
              If needed, we can change this. See CastStarted.h. */
    const Item* item{nullptr};

    /** The ID of the target entity. If this is an EntityInteraction cast, this 
        will always be present. Otherwise, this will be filled if the client 
        has a current target. */
    entt::entity targetEntity{entt::null};

    /** The target position. This will be filled if the Castable has a 
        targetToolType that selects a position.*/
    Vector3 targetPosition{};

    /** The network ID of the client performing the cast.
        If non-null, this cast belongs to a client entity.
        Only filled by the server, not used by the client. */
    NetworkID clientID{NULL_NETWORK_ID};
};

} // namespace AM
