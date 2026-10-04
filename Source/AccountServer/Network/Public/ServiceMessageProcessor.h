#pragma once

#include "AccountDefs.h"
#include "AccountServiceMessageType.h"
#include "ConnectionHandle.h"
#include "SimpleMessageFramer.h"
#include "ConsumeWorldTicketResponse.h"
#include "asio/error_code.hpp"
#include "asio/io_context.hpp"
#include "asio/thread_pool.hpp"
#include <SDL3/SDL_stdinc.h>
#include <array>
#include <functional>
#include <span>

namespace AM
{
struct ConsumeWorldTicketRequest;
struct ServiceHeartbeat;

namespace AccountServer
{
class Database;

/**
 * Processes AccountServiceMessageType messages.
 *
 * These messages are received from trusted services (world server, chat
 * server). AccountServiceEndpoint authenticates the services before any of
 * their messages reach this class.
 *
 * See ClientMessageProcessor for info on adding a message.
 */
class ServiceMessageProcessor
{
public:
    using SendCallback
        = std::function<void(ConnectionHandle, BinaryBufferSharedPtr)>;
    using DisconnectCallback
        = std::function<void(ConnectionHandle, const asio::error_code&)>;

    ServiceMessageProcessor(asio::io_context& inNetworkIoContext,
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
                                AccountServiceMessageType messageType,
                                std::span<const Uint8> messageBuffer);

private:
    //-------------------------------------------------------------------------
    // Handlers
    //-------------------------------------------------------------------------
    void handleMessage(ConnectionHandle handle,
                       const ConsumeWorldTicketRequest& message);
    void handleMessage(ConnectionHandle handle,
                       const ServiceHeartbeat& message);

    //-------------------------------------------------------------------------
    // Helpers
    //-------------------------------------------------------------------------
    /**
     * Attempts to consume the given World Server ticket.
     * Returns an appropriate response message.
     */
    ConsumeWorldTicketResponse consumeWorldTicket(
        const std::array<Uint8, SERVICE_TICKET_BYTES>& ticket);

    template<typename Message>
    void handleMessage(ConnectionHandle handle,
                       std::span<const Uint8> messageBuffer);

    asio::io_context& networkIoContext;
    asio::thread_pool& databasePool;

    Database& database;

    /** Used to send messages through the owning endpoint. */
    SendCallback sendCallback;

    /** Used to disconnect services that send invalid messages. */
    DisconnectCallback disconnectCallback;

    /** Used to serialize and frame our outgoing messages. */
    SimpleMessageFramer<AccountServiceMessageType> messageFramer;
};

} // End namespace AccountServer
} // End namespace AM
