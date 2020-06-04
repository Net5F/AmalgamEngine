#ifndef CLIENTHANDLER_H_
#define CLIENTHANDLER_H_

#include "NetworkDefs.h"
#include "Client.h"
#include "Acceptor.h"
#include "IDPool.h"
#include <memory>
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
    static constexpr unsigned int MAX_CLIENTS = 100;

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
    void receiveClientMessages(const std::shared_ptr<SDLNet_SocketSet> clientSet,
                               std::unordered_map<NetworkID, Client>& clientMap);

    Network& network;

    /** Used for generating network IDs. */
    IDPool idPool;

    std::unique_ptr<Acceptor> acceptor;

    /** The socket set used for all clients. Lets us do select()-like behavior,
        allowing our receive thread to not be constantly spinning. */
    std::shared_ptr<SDLNet_SocketSet> clientSet;

    /** Calls processClients(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;
};

} /* End namespace Server */
} /* End namespace AM */

#endif /* End CLIENTHANDLER_H_ */
