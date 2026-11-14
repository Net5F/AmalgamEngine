#pragma once

#include "ChatClientMessageType.h"
#include "AccountDefs.h"

namespace AM
{

/**
 * Sent by the client after connecting, to authenticate with the chat server.
 *
 * The chat server validates the given ticket with AccountServer and
 * responds with a ChatConnectionResponse.
 */
struct ChatConnectionRequest {
    static constexpr ChatClientMessageType MESSAGE_TYPE{
        ChatClientMessageType::ChatConnectionRequest};

    /** The single-use Chat Server ticket that the client received from
        AccountServer. */
    ServiceTicket ticket{};
};

template<typename S>
void serialize(S& serializer, ChatConnectionRequest& chatConnectionRequest)
{
    serializer.container1b(chatConnectionRequest.ticket.get());
}

} // End namespace AM
