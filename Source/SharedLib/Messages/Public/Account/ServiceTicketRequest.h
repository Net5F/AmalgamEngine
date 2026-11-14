#pragma once

#include "AccountDefs.h"
#include "AccountClientMessageType.h"

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

    /** This message holds secrets, so its buffers are wiped. */
    static constexpr bool IS_SENSITIVE{true};

    /** The service that the ticket should authorize a connection to. */
    ServiceTicketAudience audience{ServiceTicketAudience::WorldServer};

    /** The client's current account session token. */
    SessionToken accountSessionToken{};
};

template<typename S>
void serialize(S& serializer, ServiceTicketRequest& serviceTicketRequest)
{
    serializer.value1b(serviceTicketRequest.audience);
    serializer.container1b(serviceTicketRequest.accountSessionToken.get());
}

} // End namespace AM
