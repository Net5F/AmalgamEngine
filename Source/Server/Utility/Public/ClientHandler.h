#ifndef CLIENTHANDLER_H_
#define CLIENTHANDLER_H_

#include "NetworkDefs.h"
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

    /**
     * When clients are connected, this determines how long we wait in the select() call
     * before looping around to manage connections/disconnections.
     */
    static constexpr Uint32 SOCKET_RECEIVE_TIMEOUT_MS = 10;

    ClientHandler(Network& inNetwork);

    virtual ~ClientHandler();

private:
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
    void acceptNewClients(std::unordered_map<NetworkID, Client>& clientMap);

    /**
     * Erase any disconnected clients from the Network's clientMap.
     */
    void eraseDisconnectedClients(std::unordered_map<NetworkID, Client>& clientMap);

    /**
     * Used by pollForMessages, checks for new messages and pushes them into their queues.
     */
    void receiveClientMessages(std::unordered_map<NetworkID, Client>& clientMap);

    Network& network;

    /** Used for generating network IDs. */
    IDPool idPool;

    /** The socket set used for all clients. Lets us do select()-like behavior,
        allowing our receive thread to not be constantly spinning. */
    std::shared_ptr<SocketSet> clientSet;

    /** The listener that we use to accept new clients. */
    Acceptor acceptor;

    /** Calls processClients(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;
};

} // End namespace Server
} // End namespace AM

#endif /* End CLIENTHANDLER_H_ */
