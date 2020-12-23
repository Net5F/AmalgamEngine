#pragma once

#include "Input.h"
#include "CircularBuffer.h"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Client
{
/**
 * Stores state that is specific to the player entity.
 */
struct PlayerState {
public:
    /**
     * The number of input snapshots that we'll remember.
     * TODO: If this is ever an issue, we can make CircularBuffer dynamic and
     *       exponentially grow it, and remove this.
     */
    static constexpr unsigned int INPUT_HISTORY_LENGTH = 20;

    /** Tracks the player's inputs. If index 0 is the current tick, index 1
        will be the previous. */
    CircularBuffer<Input::StateArr, INPUT_HISTORY_LENGTH> inputHistory;
};

} // namespace Client
} // namespace AM
