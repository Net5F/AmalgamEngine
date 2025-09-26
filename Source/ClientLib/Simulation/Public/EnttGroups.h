#pragma once

#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "Rotation.h"
#include "Collision.h"
#include "CollisionBitSets.h"
#include "InputHistory.h"
#include "GraphicState.h"
#include "entt/entity/registry.hpp"
#include <type_traits>

namespace AM
{
namespace Client
{
/**
 * A helper class for establishing and working with the entt groups that the  
 * client uses.
 * 
 * Groups can be finicky to work with. If you establish a group <Foo, Bar, Baz>
 * and later try to get group <Foo, Baz>, it'll result in a compiler error. 
 * To make them easier to work with, we provide these functions that return the 
 * most-restrictive versions of each group.
 *
 * Other less-restrictive groups may be made by hand (e.g. <Foo, Bar> in the 
 * above example), as long as they're compatible with the constraints imposed 
 * by these groups.
 */
class EnttGroups
{
public:
    /**
     * Returns the movement group, a group used for NPC movement and entity 
     * sprite sorting.
     */
    static auto getMovementGroup(entt::registry& registry)
    {
        return registry
            .group<Input, Position, PreviousPosition, Movement,
                   MovementModifiers, Rotation, Collision, CollisionBitSets>(
                entt::get<GraphicState>, entt::exclude<InputHistory>);
    }

    /**
     * Initializes all of the entt groups that the client uses.
     */
    static void init(entt::registry& registry)
    {
        getMovementGroup(registry);
    }
};

} // namespace Client
} // End namespace AM
