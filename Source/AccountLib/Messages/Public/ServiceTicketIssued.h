#pragma once

#include "AccountDefs.h"
#include "AccountMessageType.h"
#include <array>

namespace AM
{

/**
 * Sent by the server in response to a service-ticket request.
 */
struct ServiceTicketIssued {
    // The AccountMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountMessageType MESSAGE_TYPE{
        AccountMessageType::ServiceTicketIssued};

    enum Result : Uint8 {
        Success,
        InvalidSession,
        RateLimited,
        InternalError
    };
    Result result{Result::InternalError};

    /** If result == success, this authorizes one connection attempt. */
    std::array<Uint8, SERVICE_TICKET_BYTES> ticket{};

    /** If result == success, this identifies the authorized service. */
    ServiceTicketAudience audience{ServiceTicketAudience::WorldServer};

    /** If result == success, this identifies the authorized server instance. */
    Sint64 targetServerID{0};

    /** If result == success, this is the ticket's Unix expiration timestamp. */
    Sint64 expiresAt{0};
};

template<typename S>
void serialize(S& serializer, ServiceTicketIssued& serviceTicketIssued)
{
    serializer.value1b(serviceTicketIssued.result);
    serializer.container1b(serviceTicketIssued.ticket);
    serializer.value1b(serviceTicketIssued.audience);
    serializer.value8b(serviceTicketIssued.targetServerID);
    serializer.value8b(serviceTicketIssued.expiresAt);
}

} // End namespace AM
