#include "ClientMessageProcessor.h"
#include "Config.h"
#include "Database.h"
#include "CryptoHelpers.h"
#include "Deserialize.h"
#include "RegisterRequest.h"
#include "LoginRequest.h"
#include "LogoutRequest.h"
#include "RecoverAccountRequest.h"
#include "ServiceTicketRequest.h"
#include "AccountHelpers.h"
#include "Log.h"
#include "asio/error.hpp"
#include "asio/post.hpp"
#include <algorithm>
#include <chrono>
#include <optional>
#include <utility>

namespace AM
{
namespace AccountServer
{

/**
 * Returns the current Unix timestamp, in seconds.
 */
static Sint64 getCurrentUnixTime()
{
    return static_cast<Sint64>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
}

ClientMessageProcessor::ClientMessageProcessor(
    asio::io_context& inNetworkIoContext, asio::thread_pool& inDatabasePool,
    Database& inDatabase, SendCallback inSendCallback,
    DisconnectCallback inDisconnectCallback)
: networkIoContext{inNetworkIoContext}
, databasePool{inDatabasePool}
, database{inDatabase}
, sendCallback{std::move(inSendCallback)}
, disconnectCallback{std::move(inDisconnectCallback)}
, messageFramer{Config::MAX_WRITE_PAYLOAD_SIZE}
, dummyPasswordHash{}
{
    // Generate the dummy hash.
    std::optional<std::string> generatedDummyHash{
        CryptoHelpers::hashPassword("Dummy password used for login timing.")};
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
        case AccountClientMessageType::RecoverAccountRequest: {
            handleMessage<RecoverAccountRequest>(handle, messageBuffer);
            break;
        }
        case AccountClientMessageType::ServiceTicketRequest: {
            handleMessage<ServiceTicketRequest>(handle, messageBuffer);
            break;
        }
        default: {
            LOG_INFO("Received unexpected client message type: %u",
                     static_cast<unsigned int>(messageType));
            disconnectCallback(handle, asio::error::make_error_code(
                                           asio::error::invalid_argument));
            break;
        }
    }
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           const RegisterRequest& message)
{
    // Validate the username.
    AccountHelpers::ValidateResult usernameResult{
        AccountHelpers::validateUsername(message.username)};
    if (!(usernameResult.success())) {
        sendCallback(handle, messageFramer.frameMessage(RegisterResponse{
                                 .result{RegisterResponse::InvalidUsername}}));
        return;
    }

    // Validate the password.
    AccountHelpers::ValidateResult passwordResult{
        AccountHelpers::validatePassword(message.password)};
    if (!(passwordResult.success())) {
        sendCallback(handle, messageFramer.frameMessage(RegisterResponse{
                                 .result{RegisterResponse::InvalidPassword}}));
        return;
    }

    // Attempt to register the account.
    respondFromDatabasePool(handle, [this, username = message.username,
                                     password = message.password]() {
        return registerAccount(username, password);
    });
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           const LoginRequest& message)
{
    // Attempt to create a login session for the account.
    respondFromDatabasePool(handle, [this, username = message.username,
                                     password = message.password]() {
        return authenticateAndCreateSession(username, password);
    });
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           const LogoutRequest& message)
{
    respondFromDatabasePool(handle,
                            [this, sessionToken = message.sessionToken]() {
                                return logoutSession(sessionToken);
                            });
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           const RecoverAccountRequest& message)
{
    // Validate the new password.
    AccountHelpers::ValidateResult passwordResult{
        AccountHelpers::validatePassword(message.newPassword)};
    if (!(passwordResult.success())) {
        sendCallback(handle,
                     messageFramer.frameMessage(RecoverAccountResponse{
                         .result{RecoverAccountResponse::InvalidPassword}}));
        return;
    }

    // A key of the wrong length can never match, so skip the expensive
    // password hashing.
    if (message.recoveryKey.size() != RECOVERY_KEY_CHARACTERS) {
        sendCallback(handle,
                     messageFramer.frameMessage(RecoverAccountResponse{.result{
                         RecoverAccountResponse::InvalidAccountDetails}}));
        return;
    }

    // Attempt to recover the account.
    respondFromDatabasePool(handle, [this, username = message.username,
                                     recoveryKey = message.recoveryKey,
                                     newPassword = message.newPassword]() {
        return recoverAccount(username, recoveryKey, newPassword);
    });
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           const ServiceTicketRequest& message)
{
    // Validate the audience.
    // Note: A real client will never send an invalid audience, so we treat it
    //       as a malformed message.
    if (!isValidServiceTicketAudience(message.audience)) {
        LOG_INFO("Received service ticket request with invalid audience: %u",
                 static_cast<unsigned int>(message.audience));
        disconnectCallback(handle, asio::error::make_error_code(
                                       asio::error::invalid_argument));
        return;
    }

    respondFromDatabasePool(
        handle, [this, audience = message.audience,
                 accountSessionToken = message.accountSessionToken]() {
            return issueServiceTicket(audience, accountSessionToken);
        });
}

RegisterResponse
    ClientMessageProcessor::registerAccount(const std::string& username,
                                            const std::string& password)
{
    RegisterResponse response{};

    // Generate the recovery key.
    std::string recoveryKey{CryptoHelpers::generateRecoveryKey()};

    // Hash the password.
    std::optional<std::string> passwordHash{
        CryptoHelpers::hashPassword(password)};
    if (!passwordHash) {
        LOG_ERROR("Failed to hash password.");
        response.result = RegisterResponse::InternalError;
        return response;
    }

    // Hash the recovery key.
    std::optional<std::string> recoveryKeyHash{
        CryptoHelpers::hashSecret(recoveryKey)};
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

LoginResponse ClientMessageProcessor::authenticateAndCreateSession(
    const std::string& username, const std::string& password)
{
    LoginResponse response{};

    // Get the account info we need from the database.
    Database::AccountLoginInfo loginInfo{
        database.getAccountLoginInfo(username)};
    if (loginInfo.result == Database::AccountLoginInfo::Result::DatabaseError) {
        response.result = LoginResponse::InternalError;
        return response;
    }

    // Always perform a password verification, even if the account wasn't
    // found. Otherwise, missing accounts would return much faster and expose a
    // username-enumeration timing signal.
    bool accountExists{loginInfo.result
                       == Database::AccountLoginInfo::Result::Success};
    const std::string& hashToVerify{accountExists ? loginInfo.passwordHash
                                                  : dummyPasswordHash};
    bool passwordMatches{CryptoHelpers::verifyPassword(hashToVerify, password)};

    // Use the same response for a missing account, incorrect password, or
    // inactive account so we don't reveal which check failed.
    if (!accountExists || !passwordMatches || (loginInfo.status != "active")) {
        response.result = LoginResponse::InvalidAccountDetails;
        return response;
    }

    // Generate the raw token returned to the client, then hash it for storage.
    std::array<Uint8, SESSION_TOKEN_BYTES> sessionToken{};
    CryptoHelpers::fillRandomBytes(sessionToken);
    std::optional<std::string> sessionTokenHash{
        CryptoHelpers::hashSecret(sessionToken)};
    if (!sessionTokenHash) {
        LOG_ERROR("Failed to hash account session token.");
        response.result = LoginResponse::InternalError;
        return response;
    }

    // Create the session.
    Sint64 currentTime{getCurrentUnixTime()};
    Sint64 idleExpiresAt{currentTime + Config::ACCOUNT_SESSION_IDLE_TIMEOUT_S};
    Sint64 absoluteExpiresAt{currentTime
                             + Config::ACCOUNT_SESSION_ABSOLUTE_TIMEOUT_S};

    Database::CreateSessionResult createResult{
        database.createSession(loginInfo.accountID, *sessionTokenHash,
                               idleExpiresAt, absoluteExpiresAt)};
    if (createResult == Database::CreateSessionResult::DatabaseError) {
        response.result = LoginResponse::InternalError;
        return response;
    }
    if (createResult == Database::CreateSessionResult::AccountUnavailable) {
        // The account status changed after the password was verified.
        response.result = LoginResponse::InvalidAccountDetails;
        return response;
    }

    response.accountID = loginInfo.accountID;
    response.sessionToken = sessionToken;
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
        CryptoHelpers::hashSecret(sessionToken)};
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

RecoverAccountResponse
    ClientMessageProcessor::recoverAccount(const std::string& username,
                                           const std::string& recoveryKey,
                                           const std::string& newPassword)
{
    RecoverAccountResponse response{};

    // Hash the given recovery key, so we can look it up.
    std::optional<std::string> recoveryKeyHash{
        CryptoHelpers::hashSecret(recoveryKey)};
    if (!recoveryKeyHash) {
        LOG_ERROR("Failed to hash recovery key.");
        response.result = RecoverAccountResponse::InternalError;
        return response;
    }

    // Hash the new password.
    // Note: We do this even if the key turns out to be wrong, so that every
    //       attempt takes roughly the same amount of time.
    std::optional<std::string> newPasswordHash{
        CryptoHelpers::hashPassword(newPassword)};
    if (!newPasswordHash) {
        LOG_ERROR("Failed to hash password.");
        response.result = RecoverAccountResponse::InternalError;
        return response;
    }

    // Recovery keys are single-use, so generate a replacement.
    std::string newRecoveryKey{CryptoHelpers::generateRecoveryKey()};
    std::optional<std::string> newRecoveryKeyHash{
        CryptoHelpers::hashSecret(newRecoveryKey)};
    if (!newRecoveryKeyHash) {
        LOG_ERROR("Failed to hash recovery key.");
        response.result = RecoverAccountResponse::InternalError;
        return response;
    }

    // Attempt to recover the account.
    Database::RecoverAccountResult result{database.recoverAccount(
        username, *recoveryKeyHash, *newPasswordHash, *newRecoveryKeyHash)};
    if (result == Database::RecoverAccountResult::InvalidAccountDetails) {
        response.result = RecoverAccountResponse::InvalidAccountDetails;
    }
    else if (result == Database::RecoverAccountResult::DatabaseError) {
        response.result = RecoverAccountResponse::InternalError;
    }
    else {
        // Success
        response.recoveryKey = std::move(newRecoveryKey);
        response.result = RecoverAccountResponse::Success;
    }

    return response;
}

ServiceTicketResponse ClientMessageProcessor::issueServiceTicket(
    ServiceTicketAudience audience,
    const std::array<Uint8, SESSION_TOKEN_BYTES>& accountSessionToken)
{
    // Note: The audience is echoed back even on failure, so the client can
    //       match the response to its request.
    ServiceTicketResponse response{.audience{audience}};

    // Validate (and refresh) the session that's requesting the ticket.
    std::optional<std::string> sessionTokenHash{
        CryptoHelpers::hashSecret(accountSessionToken)};
    if (!sessionTokenHash) {
        LOG_ERROR("Failed to hash account session token.");
        response.result = ServiceTicketResponse::InternalError;
        return response;
    }

    Database::AccountSessionInfo sessionInfo{database.validateSession(
        *sessionTokenHash, Config::ACCOUNT_SESSION_IDLE_TIMEOUT_S)};
    if (sessionInfo.result
        == Database::AccountSessionInfo::Result::DatabaseError) {
        response.result = ServiceTicketResponse::InternalError;
        return response;
    }
    if (sessionInfo.result
        == Database::AccountSessionInfo::Result::SessionNotFound) {
        response.result = ServiceTicketResponse::InvalidSession;
        return response;
    }

    // Generate the raw ticket returned to the client, then hash it for storage.
    std::array<Uint8, SERVICE_TICKET_BYTES> serviceTicket{};
    CryptoHelpers::fillRandomBytes(serviceTicket);
    std::optional<std::string> serviceTicketHash{
        CryptoHelpers::hashSecret(serviceTicket)};
    if (!serviceTicketHash) {
        LOG_ERROR("Failed to hash service ticket.");
        response.result = ServiceTicketResponse::InternalError;
        return response;
    }

    // The ticket can't outlive its session.
    Sint64 currentTime{getCurrentUnixTime()};
    Sint64 expiresAt{std::min(currentTime + Config::SERVICE_TICKET_LIFETIME_S,
                              sessionInfo.absoluteExpiresAt)};
    if (expiresAt <= currentTime) {
        response.result = ServiceTicketResponse::InvalidSession;
        return response;
    }

    // Create the ticket.
    Database::CreateServiceTicketResult createResult{
        database.createServiceTicket(sessionInfo.sessionID, *serviceTicketHash,
                                     audience, expiresAt)};
    if (createResult
        == Database::CreateServiceTicketResult::SessionUnavailable) {
        response.result = ServiceTicketResponse::InvalidSession;
        return response;
    }
    if (createResult == Database::CreateServiceTicketResult::DatabaseError) {
        response.result = ServiceTicketResponse::InternalError;
        return response;
    }

    response.ticket = serviceTicket;
    response.expiresAt = expiresAt;
    response.result = ServiceTicketResponse::Success;
    return response;
}

template<typename CreateResponse>
void ClientMessageProcessor::respondFromDatabasePool(
    ConnectionHandle handle, CreateResponse createResponse)
{
    asio::post(databasePool, [this, handle,
                              createResponse = std::move(createResponse)]() {
        auto response{createResponse()};

        // Send the response (must be done on the network thread).
        asio::post(
            networkIoContext, [this, handle, response = std::move(response)]() {
                sendCallback(handle, messageFramer.frameMessage(response));
            });
    });
}

template<typename Message>
void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
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
    handleMessage(handle, message);
}

} // End namespace AccountServer
} // End namespace AM
