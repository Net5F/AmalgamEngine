#pragma once

#include "AccountServiceMessageType.h"
#include <string>

namespace AM
{

/**
 * Sent by the Account Server after attempting to consume a World Server
 * connection ticket.
 */
struct ConsumeWorldTicketResponse {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::ConsumeWorldTicketResponse};

    /** Maximum serialized account-status length. */
    static constexpr std::size_t ACCOUNT_STATUS_MAX{32};

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
               ConsumeWorldTicketResponse& consumeWorldTicketResponse)
{
    serializer.value1b(consumeWorldTicketResponse.result);
    serializer.value8b(consumeWorldTicketResponse.accountID);
    serializer.value8b(consumeWorldTicketResponse.accountSessionID);
    serializer.text1b(consumeWorldTicketResponse.accountStatus,
                      ConsumeWorldTicketResponse::ACCOUNT_STATUS_MAX);
}

} // End namespace AM
