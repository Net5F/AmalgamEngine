#pragma once

#include "MovementState.h"
#include "MessageType.h"
#include "SharedConfig.h"
#include <SDL2/SDL_stdinc.h>
#include <vector>
#include "bitsery/bitsery.h"

namespace AM
{
/**
 * Sent by the server to tell a client that an entity has moved and must have
 * its state updated.
 *
 * Contains new entity movement state for a single sim tick.
 *
 * Each client is only sent the state of entities that are in their area of
 * interest.
 */
struct MovementUpdate {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE = MessageType::MovementUpdate;

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The new state of all relevant entities that updated on this tick. */
    std::vector<MovementState> movementStates;
};

template<typename S>
void serialize(S& serializer, MovementUpdate& movementUpdate)
{
    serializer.value4b(movementUpdate.tickNum);
    serializer.enableBitPacking([&movementUpdate](typename S::BPEnabledType& sbp) {
            sbp.container(movementUpdate.movementStates,
                             static_cast<std::size_t>(SharedConfig::MAX_ENTITIES));
        });
}

} // End namespace AM
