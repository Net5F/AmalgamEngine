#pragma once

#include "AccountDefs.h"
#include "AccountServiceMessageType.h"
#include <array>

namespace AM
{

/**
 * Sent by a trusted World Server to validate and consume a connection ticket.
 */
struct ConsumeWorldTicketRequest {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::ConsumeWorldTicketRequest};

    /** Chosen by the requesting service. Echoed back in the response so
        the service can match it to this request. */
    Uint32 requestID{0};

    /** The single-use ticket presented by the client. */
    std::array<Uint8, SERVICE_TICKET_BYTES> ticket{};
};

template<typename S>
void serialize(S& serializer,
               ConsumeWorldTicketRequest& consumeWorldTicketRequest)
{
    serializer.value4b(consumeWorldTicketRequest.requestID);
    serializer.container1b(consumeWorldTicketRequest.ticket);
}

} // End namespace AM
