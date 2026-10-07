#pragma once

#include "AccountDefs.h"
#include "AccountServiceMessageType.h"
#include <array>

namespace AM
{

/**
 * Sent by a trusted service (world server, chat server) to validate and
 * consume a connection ticket that a client presented to it.
 */
struct ConsumeServiceTicketRequest {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::ConsumeServiceTicketRequest};

    /** Chosen by the requesting service. Echoed back in the response so
        the service can match it to this request. */
    Uint32 requestID{0};

    /** The requesting service. The ticket is only accepted if it was issued
        for this audience. */
    ServiceTicketAudience audience{ServiceTicketAudience::WorldServer};

    /** The single-use ticket presented by the client. */
    std::array<Uint8, SERVICE_TICKET_BYTES> ticket{};
};

template<typename S>
void serialize(S& serializer,
               ConsumeServiceTicketRequest& consumeServiceTicketRequest)
{
    serializer.value4b(consumeServiceTicketRequest.requestID);
    serializer.value1b(consumeServiceTicketRequest.audience);
    serializer.container1b(consumeServiceTicketRequest.ticket);
}

} // End namespace AM
