#pragma once

#include "Input.h"
#include "Movement.h"
#include "CircularBuffer.h"

namespace AM
{
namespace Client
{
/**
 * Stores a history of inputs that have been applied to the player entity.
 *
 * Only used for the player entity. You can use this component to distinguish 
 * between player and non-player entities.
 */
struct InputHistory {
public:
    /**
     * The number of input snapshots that we'll remember.
     * TODO: If this is ever an issue, we can make CircularBuffer dynamic and
     *       exponentially grow it, and remove this.
     */
    static constexpr unsigned int LENGTH{20};

    /** Tracks the inputs that were applied to this entity.
        Increasing indices are further back in time--if index 0 is the current
        tick, index 1 is the previous tick, etc. */
    CircularBuffer<Input::StateArr, LENGTH> inputHistory;
};

} // namespace Client
} // namespace AM
