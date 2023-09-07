#pragma once

#include "EngineMessageType.h"
#include "EmptySpriteID.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by a client to request that a sprite be changed, or by the server to 
 * tell a client that an entity's sprite has changed.
 *
 * Note: We're trying to get away with using 1 message for this. If it gets 
 *       too weird for the different entity types, we can split this into 
 *       type-specific messages.
 */
struct SpriteChange {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::SpriteChange};

    // Note: We don't use this for client -> server because we don't care about
    //       syncing to the predicted client entity state. This is only sent 
    //       by the client in build mode, so it's better to do it ASAP.
    /** If this is a server -> client message, this is the server tick that 
        the change occurred on. Not used for client -> server. */
    Uint32 tickNum{0};

    /** The entity that was updated. */
    entt::entity entity{entt::null};

    /** The entity's new sprite set. */
    Uint16 spriteSetID{0};

    /** The index within spriteSet.sprites of the entity's new sprite. */
    Uint8 spriteIndex{0};
};

template<typename S>
void serialize(S& serializer, SpriteChange& spriteChange)
{
    serializer.value4b(spriteChange.tickNum);
    serializer.value4b(spriteChange.entity);
    serializer.value2b(spriteChange.spriteSetID);
    serializer.value1b(spriteChange.spriteIndex);
}

} // End namespace AM
