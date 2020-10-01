#pragma once

#include "NetworkDefs.h"
#include "ServerNetworkDefs.h"
#include "Client.h"
#include "Acceptor.h"
#include "IDPool.h"
#include <thread>
#include <unordered_map>
#include <atomic>

namespace AM
{
namespace Server
{

class Network;

/**
 * Handles all asynchronous activity that the Clients require.
 *
 * Accepts new client connections, erases clients that have been detected as disconnected,
 * and receives available messages.
 *
 * Acts directly on the Network's client map.
 */
class ClientHandler
{
public:
    /** The maximum number of clients that we will accept connections from. */
    static constexpr unsigned int MAX_CLIENTS = 100;

    ClientHandler(Network& inNetwork);

    ~ClientHandler();

private:
    /**
     * How long the accept/disconnect/receive loop in serviceClients should delay if no
     * socket activity was reported on the clientSet.
     */
    static constexpr unsigned int INACTIVE_DELAY_TIME_MS = 10;

    /**
     * Thread function, started from constructor.
     *
     * Accepts new client connections, erases clients that have been detected as disconnected,
     * and receives available messages.
     *
     * Acts directly on the Network's client map.
     */
    int serviceClients();

    /**
     * Accepts any new clients, pushing them into the Network's client map.
     */
    void acceptNewClients(ClientMap& clientMap);

    /**
     * Erase any disconnected clients from the Network's clientMap.
     */
    void eraseDisconnectedClients(ClientMap& clientMap);

    /**
     * Used by pollForMessages, checks for new messages and pushes them into their queues.
     * @return The number of messages that were received.
     */
    int receiveClientMessages(ClientMap& clientMap);

    Network& network;

    /** Used for generating network IDs. */
    IDPool idPool;

    /** The socket set used for all clients. Lets us do select()-like behavior,
        allowing our receive thread to not be constantly spinning. */
    std::shared_ptr<SocketSet> clientSet;

    /** The listener that we use to accept new clients. */
    Acceptor acceptor;

    /** A queue used for storing received messages until we can deserialize and
        route them. */
    std::queue<ClientMessage> receiveQueue;

    /** Calls processClients(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;
};

} // End namespace Server
} // End namespace AM
