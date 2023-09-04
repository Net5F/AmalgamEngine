#pragma once

#include "EngineMessageType.h"
#include <string>

namespace AM
{
/**
 * An error string, sent by the server to a client when something goes wrong.
 * Meant to be displayed to the user to give them feedback on what they tried 
 * to do.
 */
struct UserErrorString {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::UserErrorString};

    /** Used as a "we should never hit this" cap on the length of the error 
        string. Only checked in debug builds. */
    static constexpr std::size_t MAX_SCRIPT_LENGTH{500};

    /** The error string to display to the user. */
    std::string errorString{};
};

template<typename S>
void serialize(S& serializer, UserErrorString& userErrorString)
{
    serializer.text1b(userErrorString.errorString,
                      UserErrorString::MAX_SCRIPT_LENGTH);
}

} // End namespace AM
