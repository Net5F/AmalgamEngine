#pragma once

#include "AccountDefs.h"
#include "AccountClientMessageType.h"
#include <array>

namespace AM
{

/**
 * Sent by the client to request a ticket for connecting to one of our
 * services (world server, chat server).
 */
struct ServiceTicketRequest {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::ServiceTicketRequest};

    /** The service that the ticket should authorize a connection to. */
    ServiceTicketAudience audience{ServiceTicketAudience::WorldServer};

    /** The client's current account session token. */
    std::array<Uint8, SESSION_TOKEN_BYTES> accountSessionToken{};
};

template<typename S>
void serialize(S& serializer, ServiceTicketRequest& serviceTicketRequest)
{
    serializer.value1b(serviceTicketRequest.audience);
    serializer.container1b(serviceTicketRequest.accountSessionToken);
}

} // End namespace AM
