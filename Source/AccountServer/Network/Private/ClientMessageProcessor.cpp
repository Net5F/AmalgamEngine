#include "ClientMessageProcessor.h"
#include "Config.h"
#include "Database.h"
#include "Deserialize.h"
#include "Serialize.h"
#include "ByteTools.h"
#include "AccountClientMessageType.h"
#include "RegisterRequest.h"
#include "LoginRequest.h"
#include "LogoutRequest.h"
#include "RequestWorldTicket.h"
#include "AccountHelpers.h"
#include "Log.h"
#include "asio/error.hpp"
#include "asio/post.hpp"
#include "sodium.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <span>
#include <utility>

namespace AM
{
namespace AccountServer
{

ClientMessageProcessor::ClientMessageProcessor(
    asio::io_context& inNetworkIoContext, asio::thread_pool& inDatabasePool,
    Database& inDatabase, SendCallback inSendCallback,
    DisconnectCallback inDisconnectCallback)
: networkIoContext{inNetworkIoContext}
, databasePool{inDatabasePool}
, database{inDatabase}
, sendCallback{std::move(inSendCallback)}
, disconnectCallback{std::move(inDisconnectCallback)}
, dummyPasswordHash{}
{
    // Generate the dummy hash.
    std::optional<std::string> generatedDummyHash{
        hashPassword("Dummy password used for login timing.")};
    if (!generatedDummyHash) {
        LOG_FATAL("Failed to generate the dummy password hash.");
    }

    dummyPasswordHash = std::move(*generatedDummyHash);
}

void ClientMessageProcessor::processReceivedMessage(
    ConnectionHandle handle, AccountClientMessageType messageType,
    std::span<const Uint8> messageBuffer)
{
    // Match the enum values to their message types.
    switch (messageType) {
        case AccountClientMessageType::RegisterRequest: {
            handleMessage<RegisterRequest>(handle, messageBuffer);
            break;
        }
        case AccountClientMessageType::LoginRequest: {
            handleMessage<LoginRequest>(handle, messageBuffer);
            break;
        }
        case AccountClientMessageType::LogoutRequest: {
            handleMessage<LogoutRequest>(handle, messageBuffer);
            break;
        }
        case AccountClientMessageType::RequestWorldTicket: {
            handleMessage<RequestWorldTicket>(handle, messageBuffer);
            break;
        }
        default: {
            LOG_INFO("Received unexpected client message type: %u",
                     static_cast<unsigned int>(messageType));
            if (disconnectCallback) {
                disconnectCallback(
                    handle, asio::error::make_error_code(
                                asio::error::invalid_argument));
            }
            break;
        }
    }
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           const RegisterRequest& message)
{
    // Validate the username.
    AccountHelpers::ValidateResult usernameIsValid{
        AccountHelpers::validateUsername(message.username)};
    if (usernameIsValid != AccountHelpers::ValidateResult::Success) {
        sendCallback(handle, serializeMessage(RegisterResponse{
                                .result{RegisterResponse::InvalidUsername}}));
        return;
    }

    // Validate the password.
    AccountHelpers::ValidateResult passwordIsValid{
        AccountHelpers::validatePassword(message.password)};
    if (passwordIsValid != AccountHelpers::ValidateResult::Success) {
        sendCallback(handle, serializeMessage(RegisterResponse{
                                .result{RegisterResponse::InvalidPassword}}));
        return;
    }

    // Note: We use a DB worker so we don't hold up the message thread.
    asio::post(databasePool, [this, handle, username = message.username,
                              password = message.password]() {
        // Attempt to register the account.
        RegisterResponse response{registerAccount(username, password)};

        // Send the response (must be done on the network thread).
        asio::post(networkIoContext,
                   [this, handle, response = std::move(response)]() {
                       sendCallback(handle, serializeMessage(response));
                   });
    });
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           const LoginRequest& message)
{
    asio::post(databasePool, [this, handle, username = message.username,
                              password = message.password]() {
        // Attempt to create a login session for the account.
        LoginResponse response{
            authenticateAndCreateSession(username, password)};

        // Send the response (must be done on the network thread).
        asio::post(networkIoContext,
                   [this, handle, response = std::move(response)]() {
                       sendCallback(handle, serializeMessage(response));
                   });
    });
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           const LogoutRequest& message)
{
    asio::post(databasePool,
               [this, handle, sessionToken = message.sessionToken]() {
        LogoutResponse response{logoutSession(sessionToken)};

        // Send the response (must be done on the network thread).
        asio::post(networkIoContext,
                   [this, handle, response = std::move(response)]() {
                       sendCallback(handle, serializeMessage(response));
                   });
    });
}

void ClientMessageProcessor::handleMessage(
    ConnectionHandle handle, const RequestWorldTicket& message)
{
    asio::post(
        databasePool,
        [this, handle, accountSessionToken = message.accountSessionToken,
         targetServerID = message.targetServerID]() {
            ServiceTicketIssued response{
                issueWorldTicket(accountSessionToken, targetServerID)};

            // Send the response (must be done on the network thread).
            asio::post(networkIoContext,
                       [this, handle, response = std::move(response)]() {
                           sendCallback(handle, serializeMessage(response));
                       });
        });
}

RegisterResponse
    ClientMessageProcessor::registerAccount(const std::string& username,
                                            const std::string& password)
{
    RegisterResponse response{};

    // Generate the recovery key.
    std::string recoveryKey{generateRecoveryKey()};

    // Hash the password.
    std::optional<std::string> passwordHash{hashPassword(password)};
    if (!passwordHash) {
        LOG_ERROR("Failed to hash password.");
        response.result = RegisterResponse::InternalError;
        return response;
    }

    // Hash the recovery key.
    // Note: This must be 32 bytes long to match the db's key_hash column.
    std::optional<std::string> recoveryKeyHash{hashRecoveryKey(recoveryKey)};
    if (!recoveryKeyHash) {
        LOG_ERROR("Failed to hash recovery key.");
        response.result = RegisterResponse::InternalError;
        return response;
    }

    // Attempt to register the account.
    Database::RegisterResult result{
        database.registerAccount(username, *passwordHash, *recoveryKeyHash)};
    if (result == Database::RegisterResult::UsernameUnavailable) {
        response.result = RegisterResponse::UsernameUnavailable;
    }
    else if (result == Database::RegisterResult::DatabaseError) {
        response.result = RegisterResponse::InternalError;
    }
    else {
        // Success
        response.recoveryKey = std::move(recoveryKey);
        response.result = RegisterResponse::Success;
    }

    return response;
}

LoginResponse
    ClientMessageProcessor::authenticateAndCreateSession(
        const std::string& username, const std::string& password)
{
    LoginResponse response{};

    // Get the account info we need from the database.
    Database::AccountLoginInfo loginInfo{database.getAccountLoginInfo(username)};
    if (loginInfo.result
        == Database::AccountLoginInfo::Result::DatabaseError) {
        response.result = LoginResponse::InternalError;
        return response;
    }

    // Always perform a password verification, even if the account wasn't
    // found. Otherwise, missing accounts would return much faster and expose a
    // username-enumeration timing signal.
    bool accountExists{
        loginInfo.result == Database::AccountLoginInfo::Result::Success};
    const std::string& hashToVerify{
        accountExists ? loginInfo.passwordHash : dummyPasswordHash};
    bool passwordMatches{
        crypto_pwhash_str_verify(
            hashToVerify.c_str(), password.data(),
            static_cast<unsigned long long>(password.size()))
        == 0};

    // Use the same response for a missing account, incorrect password, or
    // inactive account so we don't reveal which check failed.
    if (!accountExists || !passwordMatches || (loginInfo.status != "active")) {
        response.result = LoginResponse::InvalidAccountDetails;
        return response;
    }

    // Generate the raw token returned to the client, then hash it for storage.
    std::array<Uint8, SESSION_TOKEN_BYTES> sessionToken{
        generateSessionToken()};
    std::optional<std::string> sessionTokenHash{
        hashSessionToken(sessionToken)};
    if (!sessionTokenHash) {
        LOG_ERROR("Failed to hash account session token.");
        response.result = LoginResponse::InternalError;
        return response;
    }

    // Create the session.
    Sint64 currentTime{static_cast<Sint64>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count())};
    Sint64 idleExpiresAt{
        currentTime + Config::ACCOUNT_SESSION_IDLE_TIMEOUT_S};
    Sint64 absoluteExpiresAt{
        currentTime + Config::ACCOUNT_SESSION_ABSOLUTE_TIMEOUT_S};

    Database::CreateSessionResult createResult{database.createSession(
        loginInfo.accountID, *sessionTokenHash, idleExpiresAt,
        absoluteExpiresAt)};
    if (createResult == Database::CreateSessionResult::DatabaseError) {
        response.result = LoginResponse::InternalError;
        return response;
    }
    if (createResult == Database::CreateSessionResult::AccountUnavailable) {
        // The account status changed after the password was verified.
        response.result = LoginResponse::InvalidAccountDetails;
        return response;
    }

    // Send the response.
    response.accountID = loginInfo.accountID;
    response.sessionToken = std::move(sessionToken);
    response.idleExpiresAt = idleExpiresAt;
    response.absoluteExpiresAt = absoluteExpiresAt;
    response.result = LoginResponse::Success;
    return response;
}

LogoutResponse ClientMessageProcessor::logoutSession(
    const std::array<Uint8, SESSION_TOKEN_BYTES>& sessionToken)
{
    LogoutResponse response{};

    std::optional<std::string> sessionTokenHash{
        hashSessionToken(sessionToken)};
    if (!sessionTokenHash) {
        LOG_ERROR("Failed to hash account session token.");
        response.result = LogoutResponse::InternalError;
        return response;
    }

    Database::RevokeSessionResult revokeResult{
        database.revokeSession(*sessionTokenHash)};
    if (revokeResult == Database::RevokeSessionResult::DatabaseError) {
        response.result = LogoutResponse::InternalError;
    }
    else if (revokeResult == Database::RevokeSessionResult::SessionNotFound) {
        response.result = LogoutResponse::InvalidSession;
    }
    else {
        response.result = LogoutResponse::Success;
    }

    return response;
}

ServiceTicketIssued ClientMessageProcessor::issueWorldTicket(
    const std::array<Uint8, SESSION_TOKEN_BYTES>& accountSessionToken,
    Sint64 targetServerID)
{
    ServiceTicketIssued response{};

    SessionValidation sessionValidation{
        validateSessionToken(accountSessionToken)};
    if (sessionValidation.result == SessionValidation::Result::InternalError) {
        response.result = ServiceTicketIssued::InternalError;
        return response;
    }
    if (sessionValidation.result == SessionValidation::Result::InvalidSession) {
        response.result = ServiceTicketIssued::InvalidSession;
        return response;
    }

    std::array<Uint8, SERVICE_TICKET_BYTES> serviceTicket{
        generateServiceTicket()};
    std::optional<std::string> serviceTicketHash{
        hashServiceTicket(serviceTicket)};
    if (!serviceTicketHash) {
        LOG_ERROR("Failed to hash service ticket.");
        response.result = ServiceTicketIssued::InternalError;
        return response;
    }

    Sint64 currentTime{static_cast<Sint64>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count())};
    Sint64 expiresAt{std::min(
        currentTime + Config::SERVICE_TICKET_LIFETIME_S,
        sessionValidation.absoluteExpiresAt)};
    if (expiresAt <= currentTime) {
        response.result = ServiceTicketIssued::InvalidSession;
        return response;
    }

    constexpr ServiceTicketAudience audience{
        ServiceTicketAudience::WorldServer};
    Database::CreateServiceTicketResult createResult{
        database.createServiceTicket(
            sessionValidation.sessionID, *serviceTicketHash, audience,
            targetServerID, expiresAt)};
    if (createResult
        == Database::CreateServiceTicketResult::SessionUnavailable) {
        response.result = ServiceTicketIssued::InvalidSession;
        return response;
    }
    if (createResult == Database::CreateServiceTicketResult::DatabaseError) {
        response.result = ServiceTicketIssued::InternalError;
        return response;
    }

    response.ticket = std::move(serviceTicket);
    response.audience = audience;
    response.targetServerID = targetServerID;
    response.expiresAt = expiresAt;
    response.result = ServiceTicketIssued::Success;
    return response;
}

std::string ClientMessageProcessor::generateRecoveryKey() const
{
    std::array<unsigned char, RECOVERY_KEY_RANDOM_BYTES> keyBytes{};
    randombytes_buf(keyBytes.data(), keyBytes.size());

    std::array<char, RECOVERY_KEY_BUFFER_BYTES> encodedKey{};
    sodium_bin2base64(encodedKey.data(), encodedKey.size(), keyBytes.data(),
                      keyBytes.size(),
                      sodium_base64_VARIANT_URLSAFE_NO_PADDING);

    // The encoded key is retained, so erase the raw copy.
    sodium_memzero(keyBytes.data(), keyBytes.size());

    return std::string{encodedKey.data(), RECOVERY_KEY_CHARACTERS};
}

std::optional<std::string>
    ClientMessageProcessor::hashPassword(std::string_view password) const
{
    std::array<char, crypto_pwhash_STRBYTES> encodedHash{};

    int result{crypto_pwhash_str_alg(
        encodedHash.data(), password.data(),
        static_cast<unsigned long long>(password.size()),
        crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE,
        crypto_pwhash_ALG_ARGON2ID13)};

    if (result != 0) {
        // Usually means the requested memory couldn't be allocated.
        return std::nullopt;
    }

    return std::string{encodedHash.data()};
}

std::optional<std::string>
    ClientMessageProcessor::hashRecoveryKey(
        std::string_view recoveryKey) const
{
    std::array<unsigned char, RECOVERY_KEY_HASH_BYTES> hash{};

    int result{crypto_generichash(
        hash.data(), hash.size(),
        reinterpret_cast<const unsigned char*>(recoveryKey.data()),
        static_cast<unsigned long long>(recoveryKey.size()), nullptr, 0)};

    if (result != 0) {
        return std::nullopt;
    }

    // Use the explicit-length constructor because the binary digest can
    // contain null bytes.
    return std::string{reinterpret_cast<const char*>(hash.data()), hash.size()};
}

std::array<Uint8, SESSION_TOKEN_BYTES>
    ClientMessageProcessor::generateSessionToken() const
{
    std::array<Uint8, SESSION_TOKEN_BYTES> sessionToken{};
    randombytes_buf(sessionToken.data(), sessionToken.size());
    return sessionToken;
}

std::optional<std::string> ClientMessageProcessor::hashSessionToken(
    const std::array<Uint8, SESSION_TOKEN_BYTES>& sessionToken) const
{
    std::array<unsigned char, SESSION_TOKEN_HASH_BYTES> hash{};

    int result{crypto_generichash(
        hash.data(), hash.size(), sessionToken.data(),
        static_cast<unsigned long long>(sessionToken.size()), nullptr, 0)};
    if (result != 0) {
        return std::nullopt;
    }

    // Use the explicit-length constructor because the binary digest can
    // contain null bytes.
    return std::string{reinterpret_cast<const char*>(hash.data()), hash.size()};
}

std::array<Uint8, SERVICE_TICKET_BYTES>
    ClientMessageProcessor::generateServiceTicket() const
{
    std::array<Uint8, SERVICE_TICKET_BYTES> serviceTicket{};
    randombytes_buf(serviceTicket.data(), serviceTicket.size());
    return serviceTicket;
}

std::optional<std::string> ClientMessageProcessor::hashServiceTicket(
    const std::array<Uint8, SERVICE_TICKET_BYTES>& serviceTicket) const
{
    std::array<unsigned char, SERVICE_TICKET_HASH_BYTES> hash{};

    int result{crypto_generichash(
        hash.data(), hash.size(), serviceTicket.data(),
        static_cast<unsigned long long>(serviceTicket.size()), nullptr, 0)};
    if (result != 0) {
        return std::nullopt;
    }

    // Use the explicit-length constructor because the binary digest can
    // contain null bytes.
    return std::string{reinterpret_cast<const char*>(hash.data()), hash.size()};
}

ClientMessageProcessor::SessionValidation
    ClientMessageProcessor::validateSessionToken(
        const std::array<Uint8, SESSION_TOKEN_BYTES>& sessionToken)
{
    SessionValidation validation{};

    std::optional<std::string> tokenHash{hashSessionToken(sessionToken)};
    if (!tokenHash) {
        LOG_ERROR("Failed to hash account session token.");
        validation.result = SessionValidation::Result::InternalError;
        return validation;
    }

    Database::AccountSessionInfo sessionInfo{database.validateSession(
        *tokenHash, Config::ACCOUNT_SESSION_IDLE_TIMEOUT_S)};
    if (sessionInfo.result
        == Database::AccountSessionInfo::Result::DatabaseError) {
        validation.result = SessionValidation::Result::InternalError;
        return validation;
    }
    if (sessionInfo.result
        == Database::AccountSessionInfo::Result::SessionNotFound) {
        validation.result = SessionValidation::Result::InvalidSession;
        return validation;
    }

    validation.sessionID = sessionInfo.sessionID;
    validation.accountID = sessionInfo.accountID;
    validation.createdAt = sessionInfo.createdAt;
    validation.lastUsedAt = sessionInfo.lastUsedAt;
    validation.idleExpiresAt = sessionInfo.idleExpiresAt;
    validation.absoluteExpiresAt = sessionInfo.absoluteExpiresAt;
    validation.result = SessionValidation::Result::Success;
    return validation;
}

template<typename Message>
void ClientMessageProcessor::handleMessage(
    ConnectionHandle handle, std::span<const Uint8> messageBuffer)
{
    // Deserialize the message.
    Message message{};
    if (!Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                                 message)) {
        if (disconnectCallback) {
            disconnectCallback(
                handle, asio::error::make_error_code(
                            asio::error::invalid_argument));
        }
        return;
    }

    // Pass it to the appropriate handler.
    handleMessage(handle, message);
}

template<typename Message>
BinaryBufferSharedPtr
    ClientMessageProcessor::serializeMessage(const Message& message)
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
