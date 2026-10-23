#pragma once

#include "EngineMessageType.h"
#include "Name.h"
#include "ConnectionHandle.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL3/SDL_stdinc.h>

namespace AM
{
/**
 * Used by clients to request name changes on the server.
 */
struct EntityNameChangeRequest {
    // The enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityNameChangeRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The entity to change the name of. */
    entt::entity entity{entt::null};

    /** The new name. */
    Name name{};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, EntityNameChangeRequest& entityNameChangeRequest)
{
    serializer.value4b(entityNameChangeRequest.entity);
    serializer.object(entityNameChangeRequest.name);
}

} // End namespace AM
