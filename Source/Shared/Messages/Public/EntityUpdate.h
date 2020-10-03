#pragma once

#include "Entity.h"
#include "GameDefs.h"
#include "SDL_stdinc.h"
#include <vector>
#include "bitsery/bitsery.h"

namespace AM
{

/**
 * This struct represents all new entity data for a single sim tick.
 */
struct EntityUpdate
{
    /** The tick that this EntityUpdate corresponds to. */
    Uint32 tickNum;

    /** All updated entity data. */
    std::vector<Entity> entities;
};

template <typename S>
void serialize(S& serializer, EntityUpdate& entityUpdate)
{
    serializer.value4b(entityUpdate.tickNum);
    serializer.container(entityUpdate.entities,
        static_cast<std::size_t>(MAX_ENTITIES));
}

} // End namespace AM
