#pragma once

#include "EngineMessageType.h"
#include "Position.h"
#include "ClientEntityInit.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by the client to request that an entity be created.
 */
struct EntityCreateRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityCreateRequest};

    /** The position to place the new entity at. */
    Position position{};

    // TODO: Figure out what we're doing with sprite sets.
    /** The numeric identifier for the entity's sprite. */
    int numericID{-1};
};

template<typename S>
void serialize(S& serializer, EntityCreateRequest& entityCreateRequest)
{
    serializer.object(entityCreateRequest.position);
    serializer.value4b(entityCreateRequest.numericID);
}

} // End namespace AM
