#pragma once

#include "Acceptor.h"
#include <memory>
#include <span>

namespace AM
{
namespace AccountServer 
{

/**
 * Manages client connections, handles client messaging.
 */
class ClientHandler
{
public:
    ClientHandler();

    /**
     * Process incoming and outgoing messages, and manages connections.
     */
    void processMessages();

private:
    /**
     * How long the accept/disconnect/receive loop in serviceClients should
     * delay if no socket activity was reported on the clientSet.
     */
    static constexpr unsigned int INACTIVE_DELAY_TIME_MS{1};

    /**
     * Thread function, started from constructor.
     *
     * Accepts new client connections, erases clients that have been detected as
     * disconnected, and receives available messages.
     *
     * Acts directly on the Network's client map.
     */
    void serviceClients();

    /**
     * Tries to send any messages in each client's queue over the network.
     * If a send fails, leaves the message at the front of the queue and moves
     * on to the next client's queue.
     */
    void sendClientUpdates();

    /**
     * Accepts any new clients, pushing them into the Network's client map.
     */
    void acceptNewClients(ClientMap& clientMap);

    /**
     * Erase any disconnected clients from the Network's clientMap.
     */
    void eraseDisconnectedClients(ClientMap& clientMap);

    /**
     * Receives any waiting client messages and passes them to
     * processReceivedMessage().
     */
    void receiveAndProcessClientMessages(ClientMap& clientMap);

    /**
     * Handles received client messages.
     *
     * @param client The client that we received this message from.
     * @param messageType The type of the received message.
     * @param messageBufferThe buffer that holds the message.
     */
    void processReceivedMessage(Client& client, Uint8 messageType,
                                std::span<Uint8> messageBuffer);

    /** The number of clients that are currently connected. */
    unsigned int clientCount;

    /** The socket set used for all clients. Lets us do select()-like behavior,
        allowing our receive thread to not be constantly spinning. */
    std::shared_ptr<SocketSet> clientSet;

    /** The listener that we use to accept new clients. */
    Acceptor acceptor;
};

} // End namespace AccountServer
} // End namespace AM
