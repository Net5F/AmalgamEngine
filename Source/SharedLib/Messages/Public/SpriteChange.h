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

    // Note: We don't use this for client -> server because the client only 
    //       sends this from build mode, where we don't care about perfect sync.
    /** If this is a server -> client message, this is the server tick that 
        the change occurred on. Not used for client -> server. */
    Uint32 tickNum{0};

    /** The entity that was updated. */
    entt::entity entity{entt::null};

    /** The entity's new sprite set. */
    Uint16 spriteSetID{0};

    /** The index within spriteSet.sprites of the entity's new sprite. */
    Uint8 spriteIndex{0};

    // TODO: Switch this to use character sprite sets.
    // TEMP
    /** If the entity is a client entity or NPC, this will be used instead of 
        spriteSetID/spriteIndex. */
    int spriteNumericID{EMPTY_SPRITE_ID};
    // TEMP
};

template<typename S>
void serialize(S& serializer, SpriteChange& spriteChange)
{
    serializer.value4b(spriteChange.tickNum);
    serializer.value4b(spriteChange.entity);
    serializer.value2b(spriteChange.spriteSetID);
    serializer.value1b(spriteChange.spriteIndex);
    serializer.value4b(spriteChange.spriteNumericID);
}

} // End namespace AM
