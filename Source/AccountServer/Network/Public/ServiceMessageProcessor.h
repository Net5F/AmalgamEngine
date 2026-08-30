#pragma once

#include "AccountDefs.h"
#include "AccountServiceMessageType.h"
#include "ConnectionHandle.h"
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

namespace AccountServer
{
class Database;

/**
 * Processes messages received on trusted World Server and Chat Server
 * connections.
 *
 * A connection manager must establish the peer's identity before binding its
 * receive callback to this processor.
 */
class ServiceMessageProcessor
{
public:
    using SendCallback
        = std::function<void(ConnectionHandle, BinaryBufferSharedPtr)>;
    using DisconnectCallback = std::function<void(
        ConnectionHandle, const asio::error_code&)>;

    ServiceMessageProcessor(asio::io_context& inNetworkIoContext,
                            asio::thread_pool& inDatabasePool,
                            Database& inDatabase, SendCallback sendCallback,
                            DisconnectCallback disconnectCallback);

    /** Deserializes and handles a message received from a trusted service. */
    void processReceivedMessage(ConnectionHandle handle,
                                AccountServiceMessageType messageType,
                                std::span<const Uint8> messageBuffer);

private:
    /** Must match token_hash in the service_tickets table. */
    static constexpr std::size_t SERVICE_TICKET_HASH_BYTES{32};

    void handleMessage(ConnectionHandle handle,
                       const ConsumeWorldTicketRequest& message);

    ConsumeWorldTicketResponse consumeWorldTicket(
        const std::array<Uint8, SERVICE_TICKET_BYTES>& ticket,
        Sint64 targetServerID);

    template<typename Message>
    void handleMessage(ConnectionHandle handle,
                       std::span<const Uint8> messageBuffer);

    template<typename Message>
    BinaryBufferSharedPtr serializeMessage(const Message& message);

    asio::io_context& networkIoContext;
    asio::thread_pool& databasePool;
    Database& database;
    SendCallback sendCallback;
    DisconnectCallback disconnectCallback;
};

} // End namespace AccountServer
} // End namespace AM
