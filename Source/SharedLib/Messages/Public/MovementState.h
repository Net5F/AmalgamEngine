#pragma once

#include "Input.h"
#include "Position.h"
#include "Velocity.h"
#include "Rotation.h"
#include "entt/entity/registry.hpp"
#include "bitsery/bitsery.h"

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
    Velocity velocity{};
    Rotation rotation{};
};

template<typename S>
void serialize(S& serializer, MovementState& movementState)
{
    serializer.value4b(movementState.entity);
    serializer.enableBitPacking(
        [&movementState](typename S::BPEnabledType& sbp) {
            sbp.object(movementState.input);
        });
    serializer.object(movementState.position);
    serializer.object(movementState.velocity);
    serializer.object(movementState.rotation);
}

} // End namespace AM
