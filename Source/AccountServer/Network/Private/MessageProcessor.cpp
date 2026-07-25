#include "MessageProcessor.h"
#include "Database.h"
#include "Deserialize.h"
#include "Serialize.h"
#include "ByteTools.h"
#include "AccountMessageType.h"
#include "AccountRegisterRequest.h"
#include "AccountRegisterResponse.h"
#include "AccountHelpers.h"
#include "Log.h"
#include "asio/post.hpp"
#include <span>

namespace AM
{
namespace AccountServer
{

MessageProcessor::MessageProcessor(asio::io_context& inNetworkIoContext,
                                   asio::thread_pool& inDatabasePool,
                                   Database& inDatabase,
                                   SendCallback inSendCallback)
: networkIoContext{inNetworkIoContext}
, databasePool{inDatabasePool}
, database{inDatabase}
, sendCallback{inSendCallback}
{
}

void MessageProcessor::processReceivedMessage(
    NetworkID netID, Uint8 messageType, std::span<const Uint8> messageBuffer)
{
    // Match the enum values to their message types.
    AccountMessageType accountMessageType{
        static_cast<AccountMessageType>(messageType)};
    switch (accountMessageType) {
        case AccountMessageType::AccountRegisterRequest: {
            handleMessage<AccountRegisterRequest>(netID, messageBuffer);
            break;
        }
        default: {
            LOG_FATAL("Received unexpected message type: %u", messageType);
            break;
        }
    }
}

void MessageProcessor::handleMessage(NetworkID netID,
                                     const AccountRegisterRequest& message)
{
    // Validate the username.
    AccountHelpers::ValidateResult usernameIsValid{
        AccountHelpers::validateUsername(message.username)};
    if (usernameIsValid != AccountHelpers::ValidateResult::Success) {
        sendCallback(netID, serializeMessage(AccountRegisterResponse{.result{
                                AccountRegisterResponse::InvalidUsername}}));
        return;
    }

    // Validate the password.
    AccountHelpers::ValidateResult passwordIsValid{
        AccountHelpers::validatePassword(message.password)};
    if (passwordIsValid != AccountHelpers::ValidateResult::Success) {
        sendCallback(netID, serializeMessage(AccountRegisterResponse{.result{
                                AccountRegisterResponse::InvalidPassword}}));
        return;
    }

    // Ask a DB worker to create the new account.
    asio::post(databasePool, [this, netID, username = message.username,
                              password = message.password]() mutable {
        AccountRegisterResponse response{};

        // Generate the recovery key.
        // TODO: Figure out recovery key generation.
        std::string recoveryKey{};

        // Hash the password and recovery key.
        // TODO: Integrate libsodium, hash these.
        std::string passwordHash{};
        std::string recoveryKeyHash{};

        // Attempt to register the account.
        Database::RegisterResult result{
            database.registerAccount(username, passwordHash, recoveryKeyHash)};
        if (result == Database::RegisterResult::UsernameUnavailable) {
            response.result = AccountRegisterResponse::UsernameUnavailable;
        }
        else if (result == Database::RegisterResult::DatabaseError) {
            response.result = AccountRegisterResponse::InternalError;
        }
        else {
            // Success
            response.recoveryKey = std::move(recoveryKey);
            response.result = AccountRegisterResponse::Success;
        }

        // Return to the network thread before accessing ClientManager.
        asio::post(networkIoContext,
                   [this, netID, response = std::move(response)]() mutable {
                       sendCallback(netID, serializeMessage(response));
                   });
    });
}

template<typename Message>
void MessageProcessor::handleMessage(NetworkID netID,
                                     std::span<const Uint8> messageBuffer)
{
    // Deserialize the message.
    Message message{};
    Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                            message);

    // Pass it to the appropriate handler.
    handleMessage(netID, message);
}

template<typename Message>
BinaryBufferSharedPtr MessageProcessor::serializeMessage(const Message& message)
{
    // Allocate the buffer.
    std::size_t totalMessageSize{MESSAGE_HEADER_SIZE
                                 + Serialize::measureSize(message)};
    BinaryBufferSharedPtr messageBuffer{
        std::make_shared<BinaryBuffer>(totalMessageSize)};

    // Serialize the message struct into the buffer, leaving room for the
    // header.
    std::size_t messageSize{Serialize::toBuffer(messageBuffer->data(),
                                                messageBuffer->size(), message,
                                                MESSAGE_HEADER_SIZE)};

    // Copy the type into the buffer.
    // TODO: Add a nice compile-time message if T doesn't have MESSAGE_TYPE.
    messageBuffer->at(MessageHeaderIndex::MessageType)
        = static_cast<Uint8>(Message::MESSAGE_TYPE);

    // Copy the messageSize into the buffer.
    ByteTools::write16(static_cast<Uint16>(messageSize),
                       messageBuffer->data() + MessageHeaderIndex::Size);

    return messageBuffer;
}

} // End namespace AccountServer
} // End namespace AM
