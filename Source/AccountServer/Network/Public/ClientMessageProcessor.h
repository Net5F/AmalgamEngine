#pragma once

#include "AccountDefs.h"
#include "AccountClientMessageType.h"
#include "ConnectionHandle.h"
#include "SimpleMessageFramer.h"
#include "RegisterResponse.h"
#include "LoginResponse.h"
#include "LogoutResponse.h"
#include "ServiceTicketResponse.h"
#include "asio/error_code.hpp"
#include "asio/thread_pool.hpp"
#include "asio/io_context.hpp"
#include <SDL3/SDL_stdinc.h>
#include <array>
#include <functional>
#include <span>
#include <string>

namespace AM
{
struct RegisterRequest;
struct LoginRequest;
struct LogoutRequest;
struct ServiceTicketRequest;

namespace AccountServer
{
class Database;

/**
 * Processes AccountClientMessageType messages.
 *
 * Unlike Client/WorldServer, which pass messages down to the sim layer,
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
    using DisconnectCallback
        = std::function<void(ConnectionHandle, const asio::error_code&)>;

    ClientMessageProcessor(asio::io_context& inNetworkIoContext,
                           asio::thread_pool& inDatabasePool,
                           Database& inDatabase, SendCallback inSendCallback,
                           DisconnectCallback inDisconnectCallback);

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
    //-------------------------------------------------------------------------
    // Handlers
    //-------------------------------------------------------------------------
    void handleMessage(ConnectionHandle handle, const RegisterRequest& message);

    void handleMessage(ConnectionHandle handle, const LoginRequest& message);

    void handleMessage(ConnectionHandle handle, const LogoutRequest& message);

    void handleMessage(ConnectionHandle handle,
                       const ServiceTicketRequest& message);

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
     * Attempts to issue a single-use ticket for the given service.
     *
     * Note: audience must already be validated.
     */
    ServiceTicketResponse issueServiceTicket(
        ServiceTicketAudience audience,
        const std::array<Uint8, SESSION_TOKEN_BYTES>& accountSessionToken);

    /**
     * Runs createResponse on a database worker, then sends the returned
     * response message to the given connection.
     *
     * Note: We use a database worker so we don't hold up the network thread.
     */
    template<typename CreateResponse>
    void respondFromDatabasePool(ConnectionHandle handle,
                                 CreateResponse createResponse);

    template<typename Message>
    void handleMessage(ConnectionHandle handle,
                       std::span<const Uint8> messageBuffer);

    asio::io_context& networkIoContext;
    asio::thread_pool& databasePool;

    Database& database;

    /** Used to send messages through the owning endpoint. */
    SendCallback sendCallback;

    /** Used to disconnect clients that send invalid messages. */
    DisconnectCallback disconnectCallback;

    /** Used to serialize and frame our outgoing messages. */
    SimpleMessageFramer<AccountClientMessageType> messageFramer;

    /** A valid hash that we can verify against when a login references an
        unknown account. This keeps that path close to the cost of verifying a
        real account password and reduces username-enumeration timing
        signals. */
    std::string dummyPasswordHash;
};

} // End namespace AccountServer
} // End namespace AM
