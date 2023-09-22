#pragma once

#include "ServerNetworkDefs.h"
#include "Client.h"
#include "Acceptor.h"
#include "IDPool.h"
#include "tracy/Tracy.hpp"
#include <thread>
#include <queue>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace AM
{
class EventDispatcher;

namespace Server
{
class Network;
class MessageProcessor;

/**
 * Handles all asynchronous activity that the Clients require.
 *
 * Accepts new client connections, erases clients that have been detected as
 * disconnected, and receives available messages.
 *
 * Acts directly on the Network's client map.
 */
class ClientHandler
{
public:
    ClientHandler(Network& inNetwork, EventDispatcher& inDispatcher,
                  MessageProcessor& inMessageProcessor);

    ~ClientHandler();

    /**
     * Flags the send thread to begin sending all waiting messages.
     */
    void beginSendClientUpdates();

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
     * Thread function, started from constructor.
     * Waits for beginSendClientUpdates() to flag that a send should begin.
     *
     * Tries to send any messages in each client's queue over the network.
     * If a send fails, leaves the message at the front of the queue and moves
     * on to the next client's queue.
     * If there's no messages to send, sends a heartbeat instead, with a value
     * that confirms that we've processed tick(s) with no changes to send.
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
     *
     * @return The number of messages that were received.
     */
    int receiveAndProcessClientMessages(ClientMap& clientMap);

    /**
     * Passes received client messages to the MessageProcessor.
     *
     * When a message with a tick number is received, updates the associated
     * client's tick diff data.
     *
     * @param client  The client that we received this message from.
     * @param messageType  The type of the received message.
     * @param messageSize  The length in bytes of the message in messageBuffer.
     */
    void processReceivedMessage(Client& client, Uint8 messageType,
                                std::size_t messageSize);

    /** Used to get the client map and current tick. */
    Network& network;

    /** Used to push network events like connections/disconnections. */
    EventDispatcher& dispatcher;

    /** Used to process received messages. */
    MessageProcessor& messageProcessor;

    /** Used for generating network IDs. */
    IDPool idPool;

    /** The number of clients that are currently connected. */
    unsigned int clientCount;

    /** The socket set used for all clients. Lets us do select()-like behavior,
        allowing our receive thread to not be constantly spinning. */
    std::shared_ptr<SocketSet> clientSet;

    /** The listener that we use to accept new clients. */
    Acceptor acceptor;

    /** Holds a received message while we pass it to MessageProcessor. */
    BinaryBuffer messageRecBuffer;

    /** Calls serviceClients(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the send and receive threads should end. */
    std::atomic<bool> exitRequested;

    /** Calls sendClientUpdates(). */
    std::thread sendThreadObj;
    /** Used for signaling the send thread. */
    TracyLockable(std::mutex, sendMutex);
    /** Used for signaling the send thread. */
    std::condition_variable_any sendCondVar;
    /** Used for signaling the send thread. */
    bool sendRequested;
};

} // End namespace Server
} // End namespace AM
