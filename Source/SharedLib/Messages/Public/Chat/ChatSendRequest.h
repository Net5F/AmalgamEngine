#pragma once

#include "ChatClientMessageType.h"
#include "ChatDefs.h"
#include <string>

namespace AM
{

/**
 * Sent by the client to send a chat message to the given channel.
 *
 * If the message is accepted, the chat server sends a ChatMessage to every
 * recipient (including the sender). Otherwise, it responds with a ChatError.
 */
struct ChatSendRequest {
    static constexpr ChatClientMessageType MESSAGE_TYPE{
        ChatClientMessageType::ChatSendRequest};

    /** The channel to send the message to. */
    ChatChannel channel{ChatChannel::Global};

    /** The message's text. Must pass ChatHelpers::validateChatText(). */
    std::string text{};
};

template<typename S>
void serialize(S& serializer, ChatSendRequest& chatSendRequest)
{
    serializer.value1b(chatSendRequest.channel);
    serializer.text1b(chatSendRequest.text, CHAT_MESSAGE_MAX_LENGTH);
}

} // End namespace AM
