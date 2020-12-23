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
    /** This entity's sim ID. */
    entt::entity entity{entt::null};

    Input input;
    Position position;
    Movement movement;
};

template<typename S>
void serialize(S& serializer, EntityState& entity)
{
    serializer.value4b(entity.entity);
    serializer.object(entity.input);
    serializer.object(entity.position);
    serializer.object(entity.movement);
}

} // End namespace AM
