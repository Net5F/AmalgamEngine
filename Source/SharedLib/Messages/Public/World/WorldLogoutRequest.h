#pragma once

#include "EngineMessageType.h"
#include "NetworkID.h"
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
    /** The network ID of the client that sent this message.
        Set by the server.
        No IDs are accepted from the client because we can't trust it, so we
        fill in the ID based on which socket the message came from. */
    NetworkID netID{0};
};

template<typename S>
void serialize(S& serializer, WorldLogoutRequest& worldLogoutRequest)
{
    serializer.value1b(worldLogoutRequest.type);
}

} // End namespace AM
