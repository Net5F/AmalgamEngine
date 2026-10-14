#pragma once

#include "ChatClientMessageType.h"

namespace AM
{

/**
 * Sent periodically by the client to keep its connection to the chat server
 * open, and to detect if the chat server has gone away.
 *
 * The chat server responds with a ChatHeartbeatResponse.
 */
struct ChatHeartbeat {
    static constexpr ChatClientMessageType MESSAGE_TYPE{
        ChatClientMessageType::ChatHeartbeat};
};

template<typename S>
void serialize(S&, ChatHeartbeat&)
{
}

} // End namespace AM
