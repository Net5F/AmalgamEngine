#pragma once

#include "ChatClientMessageType.h"

namespace AM
{

/**
 * Sent by the chat server in response to a ChatHeartbeat.
 */
struct ChatHeartbeatResponse {
    static constexpr ChatClientMessageType MESSAGE_TYPE{
        ChatClientMessageType::ChatHeartbeatResponse};
};

template<typename S>
void serialize(S&, ChatHeartbeatResponse&)
{
}

} // End namespace AM
