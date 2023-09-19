#pragma once

#include "EngineMessageType.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"

namespace AM
{

/**
 * Used to request a entity's init script from the server.
 *
 * Init scripts are only requested by clients for use in build mode. Only the
 * server actually runs the scripts.
 */
struct InitScriptRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::InitScriptRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The ID of the entity to get the init script for. */
    entt::entity entity{entt::null};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /**
     * The network ID of the client that sent this message.
     * Set by the server.
     * No IDs are accepted from the client because we can't trust it,
     * so we fill in the ID based on which socket the message came from.
     */
    NetworkID netID{0};
};

template<typename S>
void serialize(S& serializer, InitScriptRequest& initScriptRequest)
{
    serializer.value4b(initScriptRequest.entity);
}

} // End namespace AM
