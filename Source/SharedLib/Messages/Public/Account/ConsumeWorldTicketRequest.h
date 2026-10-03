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

    /** The single-use ticket presented by the client. */
    std::array<Uint8, SERVICE_TICKET_BYTES> ticket{};

    /** The identity of the World Server consuming the ticket. */
    Sint64 targetServerID{0};
};

template<typename S>
void serialize(S& serializer,
               ConsumeWorldTicketRequest& consumeWorldTicketRequest)
{
    serializer.container1b(consumeWorldTicketRequest.ticket);
    serializer.value8b(consumeWorldTicketRequest.targetServerID);
}

} // End namespace AM
