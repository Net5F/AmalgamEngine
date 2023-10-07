#pragma once

#include "EngineMessageType.h"
#include "NetworkDefs.h"
#include "InitScript.h"
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
 * Note: This is named "Response" to differentiate it from the InitScript 
 *       component. Normally we don't append "Response" to response messages.
 */
struct InitScriptResponse {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::InitScriptResponse};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The ID of the entity that this init script is for. */
    entt::entity entity{entt::null};

    /** This entity's init script. */
    std::string initScript{};

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
void serialize(S& serializer, InitScriptResponse& initScriptResponse)
{
    serializer.value4b(initScriptResponse.entity);
    serializer.text1b(initScriptResponse.initScript, InitScript::MAX_LENGTH);
}

} // End namespace AM
