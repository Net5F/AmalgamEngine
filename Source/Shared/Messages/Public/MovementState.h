#pragma once

#include <SDL2/SDL_stdinc.h>
#include "Input.h"
#include "Position.h"
#include "Velocity.h"
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

    Input input;
    Position position;
    Velocity velocity;
};

template<typename S>
void serialize(S& serializer, MovementState& movementState)
{
    serializer.value4b(movementState.entity);

    // Note: Input needs bit packing enabled, but we expect EntityUpdate to
    //       enable it.
    serializer.object(movementState.input);

    serializer.object(movementState.position);
    serializer.object(movementState.velocity);
}

} // End namespace AM
