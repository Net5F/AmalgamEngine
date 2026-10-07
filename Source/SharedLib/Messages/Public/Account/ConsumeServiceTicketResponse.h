#pragma once

#include "AccountServiceMessageType.h"
#include <string>

namespace AM
{

/**
 * Sent by the Account Server after attempting to consume a service connection
 * ticket.
 */
struct ConsumeServiceTicketResponse {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::ConsumeServiceTicketResponse};

    /** Maximum serialized account-status length. */
    static constexpr std::size_t ACCOUNT_STATUS_MAX{32};

    /** The requestID of the ConsumeServiceTicketRequest that this responds
        to. */
    Uint32 requestID{0};

    enum Result : Uint8 {
        Success,
        InvalidTicket,
        InternalError
    };
    Result result{Result::InternalError};

    /** If result == success, this identifies the authenticated account. */
    Sint64 accountID{0};

    /** If result == success, this identifies the owning account session. */
    Sint64 accountSessionID{0};

    /** If result == success, this is the account's current status. */
    std::string accountStatus{};
};

template<typename S>
void serialize(S& serializer,
               ConsumeServiceTicketResponse& consumeServiceTicketResponse)
{
    serializer.value4b(consumeServiceTicketResponse.requestID);
    serializer.value1b(consumeServiceTicketResponse.result);
    serializer.value8b(consumeServiceTicketResponse.accountID);
    serializer.value8b(consumeServiceTicketResponse.accountSessionID);
    serializer.text1b(consumeServiceTicketResponse.accountStatus,
                      ConsumeServiceTicketResponse::ACCOUNT_STATUS_MAX);
}

} // End namespace AM
