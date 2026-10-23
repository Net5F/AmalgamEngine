#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Sent by a client to start or cancel a logout.
 *
 * When a logout is started, the server waits Config::LOGOUT_DELAY_S before
 * removing the client's entity from the world. If the entity moves or casts
 * during this time, the logout is canceled.
 *
 * The server responds with WorldLogoutUpdate messages.
 */
struct WorldLogoutRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::WorldLogoutRequest};

    enum Type : Uint8 {
        /** Start the logout countdown. */
        Start,
        /** Cancel an ongoing logout countdown. */
        Cancel
    };

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** Whether to start or cancel a logout. */
    Type type{Type::Start};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, WorldLogoutRequest& worldLogoutRequest)
{
    serializer.value1b(worldLogoutRequest.type);
}

} // End namespace AM
