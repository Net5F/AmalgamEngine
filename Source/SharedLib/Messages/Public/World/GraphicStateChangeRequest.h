#pragma once

#include "EngineMessageType.h"
#include "GraphicState.h"
#include "ConnectionHandle.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL3/SDL_stdinc.h>

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
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer,
               GraphicStateChangeRequest& graphicStateChangeRequest)
{
    serializer.value4b(graphicStateChangeRequest.entity);
    serializer.object(graphicStateChangeRequest.graphicState);
}

} // End namespace AM
