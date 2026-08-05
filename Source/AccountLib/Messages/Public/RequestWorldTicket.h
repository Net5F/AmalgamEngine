#pragma once

#include "AccountDefs.h"
#include "AccountMessageType.h"
#include <array>

namespace AM
{

/**
 * Sent by the client to request a ticket for connecting to a World Server.
 */
struct RequestWorldTicket {
    // The AccountMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountMessageType MESSAGE_TYPE{
        AccountMessageType::RequestWorldTicket};

    /** The client's current account session token. */
    std::array<Uint8, SESSION_TOKEN_BYTES> accountSessionToken{};

    /** The World Server instance that the client wants to connect to. */
    Sint64 targetServerID{0};
};

template<typename S>
void serialize(S& serializer, RequestWorldTicket& requestWorldTicket)
{
    serializer.container1b(requestWorldTicket.accountSessionToken);
    serializer.value8b(requestWorldTicket.targetServerID);
}

} // End namespace AM
