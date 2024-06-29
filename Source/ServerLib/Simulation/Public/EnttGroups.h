#pragma once

#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "Rotation.h"
#include "Collision.h"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Server
{
class EnttGroups
{
public:
    /**
     * Initializes the entt groups that this module uses.
     *
     * These groups are the most restrictive. Other less-restrictive groups
     * may be made, as long as they're compatible with the constraints imposed
     * by these groups.
     *
     * It's useful to have them all in one spot, since the ordering of their
     * components affects what other groups are possible.
     */
    static void init(entt::registry& registry)
    {
        // movementGroup: Used for moving an entity.
        registry
            .group<Input, Position, PreviousPosition, Movement, Rotation, Collision>();
    }
};

} // End namespace Server
} // End namespace AM
