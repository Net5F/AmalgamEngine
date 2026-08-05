#include "ServiceMessageProcessor.h"
#include "AccountMessageType.h"
#include "ByteTools.h"
#include "ConsumeWorldTicketRequest.h"
#include "Database.h"
#include "Deserialize.h"
#include "Log.h"
#include "Serialize.h"
#include "asio/post.hpp"
#include "sodium.h"
#include <memory>
#include <string>
#include <utility>

namespace AM
{
namespace AccountServer
{

ServiceMessageProcessor::ServiceMessageProcessor(
    asio::io_context& inNetworkIoContext, asio::thread_pool& inDatabasePool,
    Database& inDatabase, SendCallback inSendCallback)
: networkIoContext{inNetworkIoContext}
, databasePool{inDatabasePool}
, database{inDatabase}
, sendCallback{std::move(inSendCallback)}
{
}

void ServiceMessageProcessor::processReceivedMessage(
    NetworkID netID, Uint8 messageType, std::span<const Uint8> messageBuffer)
{
    AccountMessageType accountMessageType{
        static_cast<AccountMessageType>(messageType)};
    switch (accountMessageType) {
        case AccountMessageType::ConsumeWorldTicketRequest: {
            handleMessage<ConsumeWorldTicketRequest>(netID, messageBuffer);
            break;
        }
        default: {
            LOG_FATAL("Received unexpected service message type: %u",
                      messageType);
            break;
        }
    }
}

void ServiceMessageProcessor::handleMessage(
    NetworkID netID, const ConsumeWorldTicketRequest& message)
{
    asio::post(
        databasePool,
        [this, netID, ticket = message.ticket,
         targetServerID = message.targetServerID]() {
            ConsumeWorldTicketResponse response{
                consumeWorldTicket(ticket, targetServerID)};

            asio::post(networkIoContext,
                       [this, netID, response = std::move(response)]() {
                           sendCallback(netID, serializeMessage(response));
                       });
        });
}

ConsumeWorldTicketResponse ServiceMessageProcessor::consumeWorldTicket(
    const std::array<Uint8, SERVICE_TICKET_BYTES>& ticket,
    Sint64 targetServerID)
{
    ConsumeWorldTicketResponse response{};

    std::array<unsigned char, SERVICE_TICKET_HASH_BYTES> ticketHash{};
    int hashResult{crypto_generichash(
        ticketHash.data(), ticketHash.size(), ticket.data(),
        static_cast<unsigned long long>(ticket.size()), nullptr, 0)};
    if (hashResult != 0) {
        LOG_ERROR("Failed to hash service ticket.");
        response.result = ConsumeWorldTicketResponse::InternalError;
        return response;
    }

    std::string ticketHashString{
        reinterpret_cast<const char*>(ticketHash.data()), ticketHash.size()};
    Database::ConsumedServiceTicketInfo ticketInfo{
        database.consumeServiceTicket(
            ticketHashString, ServiceTicketAudience::WorldServer,
            targetServerID)};
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
    NetworkID netID, std::span<const Uint8> messageBuffer)
{
    Message message{};
    Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                            message);
    handleMessage(netID, message);
}

template<typename Message>
BinaryBufferSharedPtr
    ServiceMessageProcessor::serializeMessage(const Message& message)
{
    std::size_t totalMessageSize{MESSAGE_HEADER_SIZE
                                 + Serialize::measureSize(message)};
    BinaryBufferSharedPtr messageBuffer{
        std::make_shared<BinaryBuffer>(totalMessageSize)};

    std::size_t messageSize{Serialize::toBuffer(
        messageBuffer->data(), messageBuffer->size(), message,
        MESSAGE_HEADER_SIZE)};

    messageBuffer->at(MessageHeaderIndex::MessageType)
        = static_cast<Uint8>(Message::MESSAGE_TYPE);
    ByteTools::write16(static_cast<Uint16>(messageSize),
                       messageBuffer->data() + MessageHeaderIndex::Size);

    return messageBuffer;
}

} // End namespace AccountServer
} // End namespace AM
