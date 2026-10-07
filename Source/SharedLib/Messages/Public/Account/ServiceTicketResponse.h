#pragma once

#include "AccountDefs.h"
#include "AccountClientMessageType.h"
#include <array>

namespace AM
{

/**
 * Sent by the server in response to a ServiceTicketRequest.
 */
struct ServiceTicketResponse {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::ServiceTicketResponse};

    enum Result : Uint8 {
        Success,
        InvalidSession,
        RateLimited,
        InternalError
    };
    Result result{Result::InternalError};

    /** If result == success, this authorizes one connection attempt. */
    std::array<Uint8, SERVICE_TICKET_BYTES> ticket{};

    /** The service that the ticket authorizes a connection to. Matches the
        request's audience, so the client can tell which request this
        responds to. */
    ServiceTicketAudience audience{ServiceTicketAudience::WorldServer};

    /** If result == success, this is the ticket's Unix expiration timestamp. */
    Sint64 expiresAt{0};
};

template<typename S>
void serialize(S& serializer, ServiceTicketResponse& serviceTicketResponse)
{
    serializer.value1b(serviceTicketResponse.result);
    serializer.container1b(serviceTicketResponse.ticket);
    serializer.value1b(serviceTicketResponse.audience);
    serializer.value8b(serviceTicketResponse.expiresAt);
}

} // End namespace AM
