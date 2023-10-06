#pragma once

#include "EngineMessageType.h"
#include <string>

namespace AM
{
/**
 * A message sent by the server, intended to show up in a client's chat window.
 * 
 * Used for various things, such as to tell a player "You must be closer to do 
 * that." when they try to interact with something.
 */
struct SystemMessage {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::SystemMessage};

    /** Used as a "we should never hit this" cap on the length of the error 
        string. Only checked in debug builds. */
    static constexpr std::size_t MAX_LENGTH{500};

    /** The message string to display to the user. */
    std::string messageString{};
};

template<typename S>
void serialize(S& serializer, SystemMessage& systemMessage)
{
    serializer.text1b(systemMessage.messageString,
                      SystemMessage::MAX_LENGTH);
}

} // End namespace AM
