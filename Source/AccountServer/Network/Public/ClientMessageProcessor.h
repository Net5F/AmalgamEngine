#pragma once

#include "AccountDefs.h"
#include "AccountClientMessageType.h"
#include "AccountServiceMessageType.h"
#include "ConnectionHandle.h"
#include "SimpleMessageFramer.h"
#include "RegisterResponse.h"
#include "LoginResponse.h"
#include "LogoutResponse.h"
#include "RecoverAccountResponse.h"
#include "ServiceTicketResponse.h"
#include "BanResponse.h"
#include "UnbanResponse.h"
#include "SetRoleResponse.h"
#include "asio/error_code.hpp"
#include "asio/thread_pool.hpp"
#include "asio/io_context.hpp"
#include <SDL3/SDL_stdinc.h>
#include <array>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace AM
{
struct RegisterRequest;
struct LoginRequest;
struct LogoutRequest;
struct RecoverAccountRequest;
struct ServiceTicketRequest;
struct BanRequest;
struct UnbanRequest;
struct SetRoleRequest;

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
    /** Sends the given message to every connected service. Must be called
        on the network thread. */
    using BroadcastCallback = std::function<void(BinaryBufferSharedPtr)>;
    using DisconnectCallback
        = std::function<void(ConnectionHandle, const asio::error_code&)>;

    ClientMessageProcessor(asio::io_context& inNetworkIoContext,
                           asio::thread_pool& inDatabasePool,
                           Database& inDatabase, SendCallback inSendCallback,
                           BroadcastCallback inBroadcastCallback,
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
                       const RecoverAccountRequest& message);

    void handleMessage(ConnectionHandle handle,
                       const ServiceTicketRequest& message);

    void handleMessage(ConnectionHandle handle, const BanRequest& message);

    void handleMessage(ConnectionHandle handle, const UnbanRequest& message);

    void handleMessage(ConnectionHandle handle, const SetRoleRequest& message);

    //-------------------------------------------------------------------------
    // Helpers
    //-------------------------------------------------------------------------
    /**
     * Attempts to register an account with the given username and password.
     * Returns an appropriate response message.
     */
    RegisterResponse registerAccount(const std::string& username,
                                     std::string_view password);

    /**
     * Attempts to create a login session for an account with the given username
     * and password.
     * Returns an appropriate response message.
     */
    LoginResponse authenticateAndCreateSession(const std::string& username,
                                               std::string_view password);

    /**
     * Attempts to revoke the account session represented by the given token.
     */
    LogoutResponse logoutSession(const SessionToken& sessionToken);

    /**
     * Attempts to use the given recovery key to set a new password on the
     * account with the given username.
     * Returns an appropriate response message.
     */
    RecoverAccountResponse recoverAccount(const std::string& username,
                                          std::string_view recoveryKey,
                                          std::string_view newPassword);

    /**
     * Attempts to issue a single-use ticket for the given service.
     *
     * Note: audience must already be validated.
     */
    ServiceTicketResponse
        issueServiceTicket(ServiceTicketAudience audience,
                           const SessionToken& accountSessionToken);

    /**
     * On a database worker, validates the requester's session. If it's valid,
     * calls moderate. Either way, sends the response to the given connection.
     *
     * @param response The response to send. Should be filled in with the
     *                 request's info, e.g. the target's username.
     * @param moderate Called as moderate(actorAccountID, response). Applies
     *                 the action and sets the response's result. If the
     *                 action affects connected clients, it should also call
     *                 broadcastNotice().
     */
    template<typename Response, typename Moderate>
    void moderateFromDatabasePool(ConnectionHandle handle,
                                  const SessionToken& accountSessionToken,
                                  Response response, Moderate moderate);

    /**
     * Sends the given notice to every connected service.
     *
     * Note: May be called from any thread.
     */
    template<typename Notice>
    void broadcastNotice(const Notice& notice);

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

    /** Used to tell our services about moderation actions. */
    BroadcastCallback broadcastCallback;

    /** Used to disconnect clients that send invalid messages. */
    DisconnectCallback disconnectCallback;

    /** Used to serialize and frame our outgoing messages. */
    SimpleMessageFramer<AccountClientMessageType> messageFramer;

    /** Used to serialize and frame the messages that we broadcast to our
        services. */
    SimpleMessageFramer<AccountServiceMessageType> serviceMessageFramer;

    /** A valid hash that we can verify against when a login references an
        unknown account. This keeps that path close to the cost of verifying a
        real account password and reduces username-enumeration timing
        signals. */
    std::string dummyPasswordHash;
};

} // End namespace AccountServer
} // End namespace AM
