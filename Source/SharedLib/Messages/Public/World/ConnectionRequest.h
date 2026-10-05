#pragma once

#include "EngineMessageType.h"
#include "AccountDefs.h"
#include <array>

namespace AM
{
/**
 * Sent by the client after connecting, to request entry into the world.
 *
 * The server validates the given ticket with the AccountServer. If it's
 * valid, the client is added to the sim. Either way, the server responds with
 * a ConnectionResponse.
 */
struct ConnectionRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ConnectionRequest};

    /** The single-use World Server ticket that the client received from the
        AccountServer. */
    std::array<Uint8, SERVICE_TICKET_BYTES> ticket{};
};

template<typename S>
void serialize(S& serializer, ConnectionRequest& connectionRequest)
{
    serializer.container1b(connectionRequest.ticket);
}

} // End namespace AM
