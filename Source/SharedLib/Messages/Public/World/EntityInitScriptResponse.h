#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
#include "EntityInitScript.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <string>

namespace AM
{

/**
 * Used to send an entity's init script to a client.
 *
 * Init scripts are only requested by clients for use in build mode. Only the
 * server actually runs the scripts.
 *
 * Note: This is named "Response" to differentiate it from the EntityInitScript
 *       component. Normally we don't append "Response" to response messages.
 */
struct EntityInitScriptResponse {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityInitScriptResponse};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The ID of the entity that this init script is for. */
    entt::entity entity{entt::null};

    /** This entity's init script. */
    EntityInitScript initScript{};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, EntityInitScriptResponse& initScriptResponse)
{
    serializer.value4b(initScriptResponse.entity);
    serializer.object(initScriptResponse.initScript);
}

} // End namespace AM
