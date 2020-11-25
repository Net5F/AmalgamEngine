#pragma once

#include "GameDefs.h"
#include "CircularBuffer.h"

namespace AM
{
namespace Client
{
/**
 * Stores data that is specific to the player entity.
 *
 * If more than just the player character uses the data, put it into a
 * component instead.
 */
struct PlayerData {
public:
    /**
     * The number of input snapshots that we'll remember.
     * TODO: If this is ever an issue, we can make CircularBuffer dynamic and
     *       exponentially grow it, and remove this.
     */
    static constexpr unsigned int INPUT_HISTORY_LENGTH = 20;

    /** The entity ID that we've been given by the server. */
    EntityID ID{0};

    /** Tracks the player's inputs. If index 0 is the current tick, index 1
        will be the previous. */
    CircularBuffer<InputStateArr, INPUT_HISTORY_LENGTH> inputHistory;

    /** Tracks whether the player has new input state that needs to be sent. */
    bool isDirty{false};
};

} // namespace Server
} // namespace AM
