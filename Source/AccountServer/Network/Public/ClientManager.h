#pragma once

#include "MessageProcessor.h"
#include "Client.h"
#include "IDPool.h"
#include "asio/ip/tcp.hpp"
#include "asio/thread_pool.hpp"
#include <memory>
#include <span>
#include <unordered_map>

namespace asio
{
class io_context;
}

namespace AM
{
namespace AccountServer 
{

/**
 * Manages client connections, handles client messaging.
 */
class ClientManager
{
public:
    ClientManager(asio::io_context& inNetworkIoContext);

    // Note: This class's events are processed by Application running 
    //       networkIoContext.

private:
    /**
     * How long the accept/disconnect/receive loop in serviceClients should
     * delay if no socket activity was reported on the clientSet.
     */
    static constexpr unsigned int INACTIVE_DELAY_TIME_MS{1};

    /**
     * Queues an async operation to accept a new client socket.
     */
    void acceptNewClient();

    /**
     * Tries to add a new client with the given socket.
     */
    void addClient(asio::ip::tcp::socket socket);

    /**
     * Erases the given client and frees netID.
     */
    void eraseDisconnectedClient(NetworkID netID, asio::error_code error);

    /**
     * Sends the given message to the client with the given netID.
     */
    void sendMessage(NetworkID netID, BinaryBufferSharedPtr message);

    /** Used for generating network IDs. */
    IDPool networkIDPool;

    /** Shared network event queue for all managers. */
    asio::io_context& networkIoContext;

    /** Used to accept new clients. */
    asio::ip::tcp::acceptor acceptor;

    /** A map of the currently-connected clients. */
    std::unordered_map<NetworkID, std::shared_ptr<Client>> clientMap;

    /** Our pool of database workers. */
    asio::thread_pool databasePool;

    MessageProcessor messageProcessor;
};

} // End namespace AccountServer
} // End namespace AM
