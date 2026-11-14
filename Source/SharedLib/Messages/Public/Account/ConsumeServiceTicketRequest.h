#pragma once

#include "AccountDefs.h"
#include "AccountServiceMessageType.h"

namespace AM
{

/**
 * Sent by a trusted service (world server, chat server) to validate and
 * consume a connection ticket that a client presented to it.
 *
 * Note: The ticket is only accepted if it was issued for the requesting
 *       service. AccountServer identifies the service by its TLS
 *       certificate.
 */
struct ConsumeServiceTicketRequest {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::ConsumeServiceTicketRequest};

    /** Chosen by the requesting service. Echoed back in the response so
        the service can match it to this request. */
    Uint32 requestID{0};

    /** The single-use ticket presented by the client. */
    ServiceTicket ticket{};

    /** If true, the response will include the account's AccountData.
        Only the world server may set this. It leaves it false if no
        components use PlayerPersistence::Account. */
    bool includeAccountData{false};
};

template<typename S>
void serialize(S& serializer,
               ConsumeServiceTicketRequest& consumeServiceTicketRequest)
{
    serializer.value4b(consumeServiceTicketRequest.requestID);
    serializer.container1b(consumeServiceTicketRequest.ticket.get());
    serializer.value1b(consumeServiceTicketRequest.includeAccountData);
}

} // End namespace AM
