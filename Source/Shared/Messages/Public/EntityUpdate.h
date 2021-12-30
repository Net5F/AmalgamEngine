#pragma once

#include "EntityState.h"
#include "MessageType.h"
#include "SharedConfig.h"
#include <SDL2/SDL_stdinc.h>
#include <vector>
#include "bitsery/bitsery.h"

namespace AM
{
/**
 * Contains new entity state for a single sim tick.
 *
 * Each client is only sent the state of entities that are in their area of
 * interest.
 */
struct EntityUpdate {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE = MessageType::EntityUpdate;

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The new state of all relevant entities that updated on this tick. */
    std::vector<EntityState> entityStates;
};

template<typename S>
void serialize(S& serializer, EntityUpdate& entityUpdate)
{
    serializer.value4b(entityUpdate.tickNum);
    serializer.enableBitPacking([&entityUpdate](typename S::BPEnabledType& sbp) {
            sbp.container(entityUpdate.entityStates,
                             static_cast<std::size_t>(SharedConfig::MAX_ENTITIES));
        });
}

} // End namespace AM
