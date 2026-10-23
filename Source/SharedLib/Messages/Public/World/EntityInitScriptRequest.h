#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"

namespace AM
{

/**
 * Used to request an entity's init script from the server.
 *
 * Init scripts are only requested by clients for use in build mode. Only the
 * server actually runs the scripts.
 */
struct EntityInitScriptRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityInitScriptRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The ID of the entity to get the init script for. */
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
void serialize(S& serializer, EntityInitScriptRequest& initScriptRequest)
{
    serializer.value4b(initScriptRequest.entity);
}

} // End namespace AM
