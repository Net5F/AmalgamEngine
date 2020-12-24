#pragma once

#include "SDL_stdinc.h"
#include "Input.h"
#include "Position.h"
#include "Movement.h"
#include "entt/entity/registry.hpp"

namespace AM
{
/**
 * This struct represents the data for a single entity.
 * Normally an entity's data lives in the ECS, so this is only really useful for
 * messaging.
 */
struct EntityState {
    /** The entity that this state belongs to. */
    entt::entity entity{entt::null};

    Input input;
    Position position;
    Movement movement;
};

template<typename S>
void serialize(S& serializer, EntityState& entityState)
{
    serializer.value4b(entityState.entity);
    serializer.object(entityState.input);
    serializer.object(entityState.position);
    serializer.object(entityState.movement);
}

} // End namespace AM
