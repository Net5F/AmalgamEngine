#include "ServiceMessageProcessor.h"
#include "Config.h"
#include "Database.h"
#include "CryptoHelpers.h"
#include "Deserialize.h"
#include "ConsumeWorldTicketRequest.h"
#include "Log.h"
#include "asio/error.hpp"
#include "asio/post.hpp"
#include <optional>
#include <string>
#include <utility>

namespace AM
{
namespace AccountServer
{

ServiceMessageProcessor::ServiceMessageProcessor(
    asio::io_context& inNetworkIoContext, asio::thread_pool& inDatabasePool,
    Database& inDatabase, SendCallback inSendCallback,
    DisconnectCallback inDisconnectCallback)
: networkIoContext{inNetworkIoContext}
, databasePool{inDatabasePool}
, database{inDatabase}
, sendCallback{std::move(inSendCallback)}
, disconnectCallback{std::move(inDisconnectCallback)}
, messageFramer{Config::MAX_WRITE_PAYLOAD_SIZE}
{
}

void ServiceMessageProcessor::processReceivedMessage(
    ConnectionHandle handle, AccountServiceMessageType messageType,
    std::span<const Uint8> messageBuffer)
{
    // Match the enum values to their message types.
    switch (messageType) {
        case AccountServiceMessageType::ConsumeWorldTicketRequest: {
            handleMessage<ConsumeWorldTicketRequest>(handle, messageBuffer);
            break;
        }
        default: {
            LOG_INFO("Received unexpected service message type: %u",
                     static_cast<unsigned int>(messageType));
            disconnectCallback(handle, asio::error::make_error_code(
                                           asio::error::invalid_argument));
            break;
        }
    }
}

void ServiceMessageProcessor::handleMessage(
    ConnectionHandle handle, const ConsumeWorldTicketRequest& message)
{
    // Note: We use a DB worker so we don't hold up the network thread.
    asio::post(databasePool, [this, handle, ticket = message.ticket]() {
        ConsumeWorldTicketResponse response{consumeWorldTicket(ticket)};

        // Send the response (must be done on the network thread).
        asio::post(
            networkIoContext, [this, handle, response = std::move(response)]() {
                sendCallback(handle, messageFramer.frameMessage(response));
            });
    });
}

ConsumeWorldTicketResponse ServiceMessageProcessor::consumeWorldTicket(
    const std::array<Uint8, SERVICE_TICKET_BYTES>& ticket)
{
    ConsumeWorldTicketResponse response{};

    std::optional<std::string> ticketHash{CryptoHelpers::hashSecret(ticket)};
    if (!ticketHash) {
        LOG_ERROR("Failed to hash service ticket.");
        response.result = ConsumeWorldTicketResponse::InternalError;
        return response;
    }

    Database::ConsumedServiceTicketInfo ticketInfo{
        database.consumeServiceTicket(*ticketHash,
                                      ServiceTicketAudience::WorldServer)};
    if (ticketInfo.result
        == Database::ConsumedServiceTicketInfo::Result::DatabaseError) {
        response.result = ConsumeWorldTicketResponse::InternalError;
        return response;
    }
    if (ticketInfo.result
        == Database::ConsumedServiceTicketInfo::Result::TicketNotFound) {
        response.result = ConsumeWorldTicketResponse::InvalidTicket;
        return response;
    }

    response.accountID = ticketInfo.accountID;
    response.accountSessionID = ticketInfo.accountSessionID;
    response.accountStatus = std::move(ticketInfo.accountStatus);
    response.result = ConsumeWorldTicketResponse::Success;
    return response;
}

template<typename Message>
void ServiceMessageProcessor::handleMessage(
    ConnectionHandle handle, std::span<const Uint8> messageBuffer)
{
    // Deserialize the message.
    Message message{};
    if (!Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                                 message)) {
        disconnectCallback(handle, asio::error::make_error_code(
                                       asio::error::invalid_argument));
        return;
    }

    // Pass it to the appropriate handler.
    handleMessage(handle, message);
}

} // End namespace AccountServer
} // End namespace AM
