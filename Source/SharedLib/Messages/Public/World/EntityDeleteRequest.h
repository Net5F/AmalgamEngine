#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL3/SDL_stdinc.h>

namespace AM
{
/**
 * Sent by the client to request that an entity be deleted.
 */
struct EntityDeleteRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityDeleteRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The entity to delete. */
    entt::entity entity{entt::null};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, EntityDeleteRequest& entityDeleteRequest)
{
    serializer.value4b(entityDeleteRequest.entity);
}

} // End namespace AM
