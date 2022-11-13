#pragma once

#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Rotation.h"
#include "Collision.h"
#include "InputHistory.h"
#include "Ignore.h"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Client
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
        // Used for NPC movement and dynamic entity sorting.
        auto movementGroup
            = registry.group<Input, Position, PreviousPosition, Velocity, Rotation,
                             Collision>(entt::get<Sprite>, entt::exclude<InputHistory>);
        ignore(movementGroup);
    }
};

} // namespace Client
} // End namespace AM
