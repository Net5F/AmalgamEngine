#pragma once

#include "Input.h"
#include "Position.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "entt/entity/registry.hpp"

namespace AM
{
/**
 * Contains movement state data for a single entity.
 *
 * Used for sending movement state updates to clients.
 */
struct MovementState {
    /** The entity that this state belongs to. */
    entt::entity entity{entt::null};

    Input input{};
    Position position{};
    Movement movement{};
    MovementModifiers movementMods{};

    // Note: Rotation is calculated client-side.
};

template<typename S>
void serialize(S& serializer, MovementState& movementState)
{
    serializer.value4b(movementState.entity);
    serializer.object(movementState.input);
    serializer.object(movementState.position);
    serializer.object(movementState.movement);
    serializer.object(movementState.movementMods);
}

} // End namespace AM
