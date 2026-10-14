#pragma once

#include "ChatClientMessageType.h"
#include "ChatDefs.h"
#include "AccountDefs.h"
#include <string>

namespace AM
{

/**
 * Sent by the chat server to deliver a chat message to a client.
 */
struct ChatMessage {
    static constexpr ChatClientMessageType MESSAGE_TYPE{
        ChatClientMessageType::ChatMessage};

    /** The channel that the message was sent to. */
    ChatChannel channel{ChatChannel::Global};

    /** The username of the account that sent the message. */
    std::string senderName{};

    /** The message's text. */
    std::string text{};
};

template<typename S>
void serialize(S& serializer, ChatMessage& chatMessage)
{
    serializer.value1b(chatMessage.channel);
    serializer.text1b(chatMessage.senderName, USERNAME_MAX_LENGTH);
    serializer.text1b(chatMessage.text, CHAT_MESSAGE_MAX_LENGTH);
}

} // End namespace AM
