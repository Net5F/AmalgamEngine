#pragma once

#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "BoundingBox.h"
#include "Sprite.h"
#include "ClientSimData.h"
#include "PositionHasChanged.h"
#include "Name.h"
#include "Ignore.h"
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
    static void init(entt::registry& registry) {
        // Used for moving an entity.
        // Sprite is needed to get the model-space bounding box so we can
        // move BoundingBox.
        auto movementGroup = registry.group<Input, Position, PreviousPosition, Velocity,
            Sprite, BoundingBox>();
        ignore(movementGroup);
    }
};

} // End namespace Server
} // End namespace AM
