#pragma once

#include "EngineMessageType.h"
#include "GraphicState.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Used by clients to request graphic state changes on the server.
 */
struct GraphicStateChangeRequest {
    // The enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::GraphicStateChangeRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The entity to change the graphic state of. */
    entt::entity entity{entt::null};

    /** The new graphic state. */
    GraphicState graphicState;

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
void serialize(S& serializer,
               GraphicStateChangeRequest& graphicStateChangeRequest)
{
    serializer.value4b(graphicStateChangeRequest.entity);
    serializer.object(graphicStateChangeRequest.graphicState);
}

} // End namespace AM
