#pragma once

#include <SDL2/SDL_stdinc.h>
#include "Input.h"
#include "Position.h"
#include "Movement.h"
#include "entt/entity/registry.hpp"

namespace AM
{
/**
 * Contains state data for a single entity.
 *
 * Used for sending state updates to clients.
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

    serializer.enableBitPacking([&entityState](typename S::BPEnabledType& sbp) {
            sbp.object(entityState.input);
        });

    serializer.object(entityState.position);
    serializer.object(entityState.movement);
}

} // End namespace AM
