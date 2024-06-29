#pragma once

#include "Input.h"
#include "Position.h"
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
    // Note: velocityZ is all we need to network from the Movement component.
    float movementVelocityZ{};
    // Note: Rotation is calculated client-side.
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
    serializer.value4b(movementState.movementVelocityZ);
}

} // End namespace AM
