#pragma once

#include "ChatServiceMessageType.h"

namespace AM
{

/**
 * Sent by the chat server in response to a ChatServiceHeartbeat.
 */
struct ChatServiceHeartbeatResponse {
    static constexpr ChatServiceMessageType MESSAGE_TYPE{
        ChatServiceMessageType::ChatServiceHeartbeatResponse};
};

template<typename S>
void serialize(S&, ChatServiceHeartbeatResponse&)
{
}

} // End namespace AM
