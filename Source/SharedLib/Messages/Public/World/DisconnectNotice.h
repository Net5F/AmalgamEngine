#pragma once

#include "EngineMessageType.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Sent by the server right before it disconnects a client, to tell the client
 * why.
 *
 * This is always the last message that the client will receive on its
 * connection.
 */
struct DisconnectNotice {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::DisconnectNotice};

    enum Reason : Uint8 {
        /** The client's logout completed. */
        Logout,
        /** The client's account logged in from another connection. */
        LoggedInElsewhere,
        /** The client was kicked. */
        Kicked,
        /** The client's account was banned. */
        Banned,
        /** The server is shutting down. */
        ServerShutdown
    };

    /** Why the client is being disconnected. */
    Reason reason{Reason::Kicked};
};

template<typename S>
void serialize(S& serializer, DisconnectNotice& disconnectNotice)
{
    serializer.value1b(disconnectNotice.reason);
}

} // End namespace AM
