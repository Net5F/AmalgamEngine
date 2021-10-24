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
 * Contains all new entity data for a single sim tick.
 */
struct EntityUpdate {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE = MessageType::EntityUpdate;

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** Data for all of the entities that updated on this tick. */
    std::vector<EntityState> entityStates;
};

template<typename S>
void serialize(S& serializer, EntityUpdate& entityUpdate)
{
    serializer.value4b(entityUpdate.tickNum);
    serializer.container(entityUpdate.entityStates,
                         static_cast<std::size_t>(SharedConfig::MAX_ENTITIES));
}

} // End namespace AM
