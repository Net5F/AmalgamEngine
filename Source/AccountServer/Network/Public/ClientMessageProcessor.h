#pragma once

#include "AccountDefs.h"
#include "AccountClientMessageType.h"
#include "ConnectionHandle.h"
#include "RegisterResponse.h"
#include "LoginResponse.h"
#include "LogoutResponse.h"
#include "ServiceTicketIssued.h"
#include "asio/error_code.hpp"
#include "asio/thread_pool.hpp"
#include "asio/io_context.hpp"
#include <SDL3/SDL_stdinc.h>
#include <array>
#include <functional>
#include <span>
#include <optional>
#include <string>
#include <string_view>

namespace AM
{
struct RegisterRequest;
struct LoginRequest;
struct LogoutRequest;
struct RequestWorldTicket;

namespace AccountServer
{
class Database;

/**
 * Processes AccountClientMessageType messages.
 *
 * Unlike Client/Server, which pass messages down to the sim layer,
 * AccountServer handles messages directly in the network thread. As such,
 * these functions are actual message handlers, instead of just dispatchers.
 *
 * To add a message:
 *   1. Add #include "MyNewMessage.h" to ClientMessageProcessor.cpp.
 *   2. Add a case to the switch statement in processReceivedMessage().
 *   3. Add a forward declaration and a matching handleMessage() function.
 */
class ClientMessageProcessor
{
public:
    using SendCallback
        = std::function<void(ConnectionHandle, BinaryBufferSharedPtr)>;
    using DisconnectCallback = std::function<void(
        ConnectionHandle, const asio::error_code&)>;

    ClientMessageProcessor(asio::io_context& inNetworkIoContext,
                           asio::thread_pool& inDatabasePool,
                           Database& inDatabase, SendCallback sendCallback,
                           DisconnectCallback disconnectCallback);

    /**
     * Deserializes and handles received messages.
     *
     * @param handle The connection that the message came from.
     * @param messageType The type of the received message.
     * @param messageBuffer A buffer containing a serialized message, starting
     * at index 0.
     */
    void processReceivedMessage(ConnectionHandle handle,
                                AccountClientMessageType messageType,
                                std::span<const Uint8> messageBuffer);

private:
    /** Encoding 18 random bytes as unpadded Base64URL produces a 24-
        character recovery key with 144 bits of entropy. */
    static constexpr std::size_t RECOVERY_KEY_RANDOM_BYTES{18};
    /** The recovery key, plus 1 for null terminator. */
    static constexpr std::size_t RECOVERY_KEY_BUFFER_BYTES{
        RECOVERY_KEY_CHARACTERS + 1};

    /** How long the hash of the recovery key should be.
        Must match key_hash in the account_recovery_keys table. */
    static constexpr std::size_t RECOVERY_KEY_HASH_BYTES{32};

    /** How long the hash of an account session token should be.
        Must match token_hash in the account_sessions table. */
    static constexpr std::size_t SESSION_TOKEN_HASH_BYTES{32};

    /** How long the hash of a service ticket should be.
        Must match token_hash in the service_tickets table. */
    static constexpr std::size_t SERVICE_TICKET_HASH_BYTES{32};

    //-------------------------------------------------------------------------
    // Handlers
    //-------------------------------------------------------------------------
    void handleMessage(ConnectionHandle handle, const RegisterRequest& message);

    void handleMessage(ConnectionHandle handle, const LoginRequest& message);

    void handleMessage(ConnectionHandle handle, const LogoutRequest& message);

    void handleMessage(ConnectionHandle handle,
                       const RequestWorldTicket& message);

    //-------------------------------------------------------------------------
    // Helpers
    //-------------------------------------------------------------------------
    /**
     * Attempts to register an account with the given username and password.
     * Returns an appropriate response message.
     */
    RegisterResponse registerAccount(const std::string& username,
                                     const std::string& password);

    /**
     * Attempts to create a login session for an account with the given username
     * and password.
     * Returns an appropriate response message.
     */
    LoginResponse authenticateAndCreateSession(const std::string& username,
                                               const std::string& password);

    /**
     * Attempts to revoke the account session represented by the given token.
     */
    LogoutResponse logoutSession(
        const std::array<Uint8, SESSION_TOKEN_BYTES>& sessionToken);

    /**
     * Attempts to issue a single-use ticket for the requested World Server.
     */
    ServiceTicketIssued issueWorldTicket(
        const std::array<Uint8, SESSION_TOKEN_BYTES>& accountSessionToken,
        Sint64 targetServerID);

    /**
     * Generates an account recovery key.
     */
    std::string generateRecoveryKey() const;

    /**
     * Hashes the given password using argon2id.
     */
    std::optional<std::string> hashPassword(std::string_view password) const;

    /**
     * Hashes a recovery key into exactly 32 binary bytes.
     */
    std::optional<std::string>
        hashRecoveryKey(std::string_view recoveryKey) const;

    /**
     * Generates a cryptographically random account session token.
     */
    std::array<Uint8, SESSION_TOKEN_BYTES> generateSessionToken() const;

    /**
     * Hashes an account session token into exactly 32 binary bytes.
     */
    std::optional<std::string> hashSessionToken(
        const std::array<Uint8, SESSION_TOKEN_BYTES>& sessionToken) const;

    /**
     * Generates a cryptographically random service ticket.
     */
    std::array<Uint8, SERVICE_TICKET_BYTES> generateServiceTicket() const;

    /**
     * Hashes a service ticket into exactly 32 binary bytes.
     */
    std::optional<std::string> hashServiceTicket(
        const std::array<Uint8, SERVICE_TICKET_BYTES>& serviceTicket) const;

    struct SessionValidation {
        enum class Result {
            Success,
            InvalidSession,
            InternalError
        };

        Result result{Result::InternalError};
        Sint64 sessionID{0};
        Sint64 accountID{0};
        Sint64 createdAt{0};
        Sint64 lastUsedAt{0};
        Sint64 idleExpiresAt{0};
        Sint64 absoluteExpiresAt{0};
    };
    /**
     * Hashes and validates an account session token.
     *
     * Valid sessions have their last-used time and idle expiration refreshed
     * by the database.
     */
    SessionValidation validateSessionToken(
        const std::array<Uint8, SESSION_TOKEN_BYTES>& sessionToken);

    template<typename Message>
    void handleMessage(ConnectionHandle handle,
                       std::span<const Uint8> messageBuffer);

    template<typename Message>
    BinaryBufferSharedPtr serializeMessage(const Message& message);

    asio::io_context& networkIoContext;
    asio::thread_pool& databasePool;

    Database& database;

    /** Used to send messages through the owning endpoint. */
    SendCallback sendCallback;

    /** Used to disconnect clients that send invalid messages. */
    DisconnectCallback disconnectCallback;

    /** A valid hash that we can verify against when a login references an
        unknown account. This keeps that path close to the cost of verifying a
        real account password and reduces username-enumeration timing
        signals. */
    std::string dummyPasswordHash;
};

} // End namespace AccountServer
} // End namespace AM
