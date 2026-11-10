#include "ServiceMessageProcessor.h"
#include "Config.h"
#include "Database.h"
#include "CryptoHelpers.h"
#include "Deserialize.h"
#include "ConsumeServiceTicketRequest.h"
#include "ServiceHeartbeat.h"
#include "ServiceHeartbeatResponse.h"
#include "SaveAccountDataRequest.h"
#include "SaveAccountDataResponse.h"
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
, messageFramer{Config::SERVICE_MAX_WRITE_PAYLOAD_SIZE}
{
}

void ServiceMessageProcessor::processReceivedMessage(
    ConnectionHandle handle, ServiceTicketAudience service,
    AccountServiceMessageType messageType, std::span<const Uint8> messageBuffer)
{
    // Match the enum values to their message types.
    switch (messageType) {
        case AccountServiceMessageType::ConsumeServiceTicketRequest: {
            handleMessage<ConsumeServiceTicketRequest>(handle, service,
                                                       messageBuffer);
            break;
        }
        case AccountServiceMessageType::ServiceHeartbeat: {
            handleMessage<ServiceHeartbeat>(handle, service, messageBuffer);
            break;
        }
        case AccountServiceMessageType::SaveAccountDataRequest: {
            handleMessage<SaveAccountDataRequest>(handle, service,
                                                  messageBuffer);
            break;
        }
        default: {
            LOG_WARN("Received unexpected service message type: %u",
                     static_cast<unsigned int>(messageType));
            disconnectCallback(handle, asio::error::make_error_code(
                                           asio::error::invalid_argument));
            break;
        }
    }
}

void ServiceMessageProcessor::handleMessage(
    ConnectionHandle handle, ServiceTicketAudience service,
    const ConsumeServiceTicketRequest& message)
{
    if (message.includeAccountData
        && !checkAccountDataAccess(handle, service)) {
        return;
    }

    // Note: We use a DB worker so we don't hold up the network thread.
    // Note: The requesting service is the audience, so a service can only
    //       consume tickets that were issued for it.
    asio::post(
        databasePool, [this, handle, requestID = message.requestID, service,
                       ticket = message.ticket,
                       includeAccountData = message.includeAccountData]() {
            ConsumeServiceTicketResponse response{
                consumeServiceTicket(service, ticket, includeAccountData)};
            response.requestID = requestID;

            // Send the response (must be done on the network thread).
            asio::post(networkIoContext, [this, handle,
                                          response = std::move(response)]() {
                sendCallback(handle, messageFramer.frameMessage(response));
            });
        });
}

void ServiceMessageProcessor::handleMessage(ConnectionHandle handle,
                                            ServiceTicketAudience,
                                            const ServiceHeartbeat&)
{
    // Respond so the service knows we're still here.
    sendCallback(handle,
                 messageFramer.frameMessage(ServiceHeartbeatResponse{}));
}

void ServiceMessageProcessor::handleMessage(ConnectionHandle handle,
                                            ServiceTicketAudience service,
                                            SaveAccountDataRequest& message)
{
    if (!checkAccountDataAccess(handle, service)) {
        return;
    }

    // Note: We use a DB worker so we don't hold up the network thread.
    // Note: Saves for the same account may be processed out of order, since
    //       the pool has multiple workers. The world server avoids this by
    //       only having 1 save in flight per account.
    asio::post(databasePool, [this, handle, requestID = message.requestID,
                              accountID = message.accountID,
                              accountData = std::move(message.accountData)]() {
        SaveAccountDataResponse response{.requestID{requestID}};
        switch (database.saveAccountData(accountID, accountData)) {
            case Database::SaveAccountDataResult::Success: {
                response.result = SaveAccountDataResponse::Success;
                break;
            }
            case Database::SaveAccountDataResult::AccountNotFound: {
                LOG_WARN("Tried to save data for unknown account: %lld",
                         static_cast<long long>(accountID));
                response.result = SaveAccountDataResponse::InvalidAccount;
                break;
            }
            default: {
                response.result = SaveAccountDataResponse::InternalError;
                break;
            }
        }

        // Send the response (must be done on the network thread).
        asio::post(networkIoContext, [this, handle, response]() {
            sendCallback(handle, messageFramer.frameMessage(response));
        });
    });
}

ConsumeServiceTicketResponse ServiceMessageProcessor::consumeServiceTicket(
    ServiceTicketAudience audience,
    const std::array<Uint8, SERVICE_TICKET_BYTES>& ticket,
    bool includeAccountData)
{
    ConsumeServiceTicketResponse response{};

    std::optional<std::string> ticketHash{CryptoHelpers::hashSecret(ticket)};
    if (!ticketHash) {
        LOG_ERROR("Failed to hash service ticket.");
        response.result = ConsumeServiceTicketResponse::InternalError;
        return response;
    }

    // Note: This only matches tickets that were issued for this audience, so
    //       a ticket for one service can't be used to connect to another.
    Database::ConsumedServiceTicketInfo ticketInfo{
        database.consumeServiceTicket(*ticketHash, audience)};
    if (ticketInfo.result
        == Database::ConsumedServiceTicketInfo::Result::DatabaseError) {
        response.result = ConsumeServiceTicketResponse::InternalError;
        return response;
    }
    if (ticketInfo.result
        == Database::ConsumedServiceTicketInfo::Result::TicketNotFound) {
        response.result = ConsumeServiceTicketResponse::InvalidTicket;
        return response;
    }

    // If requested, get the account's saved data.
    // Note: If this fails, the ticket has already been consumed. The client
    //       will need to request a new one.
    if (includeAccountData) {
        Database::GetAccountDataResult accountDataResult{
            database.getAccountData(ticketInfo.accountID,
                                    response.accountData)};
        if (accountDataResult
            == Database::GetAccountDataResult::DatabaseError) {
            response.result = ConsumeServiceTicketResponse::InternalError;
            return response;
        }
        if (!(response.accountData.isWithinSizeLimits())) {
            LOG_ERROR("Saved account data exceeds size limit. AccountID: %lld",
                      static_cast<long long>(ticketInfo.accountID));
            response.accountData = {};
            response.result = ConsumeServiceTicketResponse::InternalError;
            return response;
        }
        response.hasAccountData
            = (accountDataResult == Database::GetAccountDataResult::Success);
    }

    response.accountID = ticketInfo.accountID;
    response.accountSessionID = ticketInfo.accountSessionID;
    response.accountStatus = std::move(ticketInfo.accountStatus);
    response.username = std::move(ticketInfo.username);
    response.result = ConsumeServiceTicketResponse::Success;
    return response;
}

bool ServiceMessageProcessor::checkAccountDataAccess(
    ConnectionHandle handle, ServiceTicketAudience service)
{
    if (service != ServiceTicketAudience::WorldServer) {
        LOG_WARN("Service tried to access account data: %s",
                 getServiceTicketAudienceName(service));
        disconnectCallback(handle, asio::error::make_error_code(
                                       asio::error::invalid_argument));
        return false;
    }

    return true;
}

template<typename Message>
void ServiceMessageProcessor::handleMessage(
    ConnectionHandle handle, ServiceTicketAudience service,
    std::span<const Uint8> messageBuffer)
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
    // Note: Handlers may move out of the message, to avoid copying large
    //       data.
    handleMessage(handle, service, message);
}

} // End namespace AccountServer
} // End namespace AM
