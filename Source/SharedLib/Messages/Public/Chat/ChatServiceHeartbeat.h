#pragma once

#include "ChatServiceMessageType.h"

namespace AM
{

/**
 * Sent periodically by a service to keep its connection to the chat server
 * open, and to detect if the chat server has gone away.
 *
 * The chat server responds with a ChatServiceHeartbeatResponse.
 */
struct ChatServiceHeartbeat {
    static constexpr ChatServiceMessageType MESSAGE_TYPE{
        ChatServiceMessageType::ChatServiceHeartbeat};
};

template<typename S>
void serialize(S&, ChatServiceHeartbeat&)
{
}

} // End namespace AM
