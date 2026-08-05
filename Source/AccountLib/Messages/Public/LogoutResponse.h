#pragma once

#include "AccountMessageType.h"

namespace AM
{

/**
 * Sent by the server in response to a logout request.
 */
struct LogoutResponse {
    // The AccountMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountMessageType MESSAGE_TYPE{
        AccountMessageType::LogoutResponse};

    enum Result : Uint8 {
        Success,
        InvalidSession,
        InternalError
    };
    Result result{Result::InternalError};
};

template<typename S>
void serialize(S& serializer, LogoutResponse& logoutResponse)
{
    serializer.value1b(logoutResponse.result);
}

} // End namespace AM
