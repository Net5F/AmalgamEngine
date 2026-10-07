#pragma once

#include "ClientMap.h"
#include "Client.h"
#include "Acceptor.h"
#include "IDPool.h"
#include "ConnectionResponse.h"
#include "ConsumeServiceTicketResponse.h"
#include "readerwriterqueue.h"
#include "tracy/Tracy.hpp"
#include <thread>
#include <queue>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <span>
#include <condition_variable>

namespace AM
{
class EventDispatcher;

namespace WorldServer
{
class WorldClientEndpoint;
class MessageProcessor;
class AccountServiceEndpoint;

/**
 * Handles all asynchronous activity that the Clients require.
 *
 * Accepts new client connections, authenticates them, erases clients that have
 * been detected as disconnected, and receives available messages.
 *
 * Newly accepted clients must send a ConnectionRequest containing a World
 * Server ticket. We validate the ticket with the AccountServer, and only tell
 * the sim about the client once it succeeds. Until then, any other messages
 * from the client are dropped.
 *
 * Acts directly on the WorldClientEndpoint's client map.
 */
class ClientHandler
{
public:
    ClientHandler(WorldClientEndpoint& inEndpoint,
                  EventDispatcher& inDispatcher,
                  MessageProcessor& inMessageProcessor,
                  AccountServiceEndpoint& inAccountEndpoint);

    ~ClientHandler();

    /**
     * Flags the send thread to begin sending all waiting messages.
     */
    void beginSendClientUpdates();

private:
    /** Extra room so that we don't run into reuse issues when almost all IDs
        are reserved.
        Note: Eventually, we should determine when old IDs are guaranteed to
              be safe to reuse (maybe after 1 tick?). This needs to also
              guarantee that the ID will be cleared from any clients. */
    static constexpr std::size_t ID_SAFETY_BUFFER{100};

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
     * Acts directly on the WorldClientEndpoint's client map.
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
     * Accepts any new clients, pushing them into the endpoint's client map.
     */
    void acceptNewClients(ClientMap& clientMap);

    /**
     * Processes any ticket validation results that the AccountServer has sent
     * us, authenticating or rejecting the associated clients.
     */
    void processTicketResults(ClientMap& clientMap);

    /**
     * Erase any disconnected clients from the endpoint's client map.
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
     * @param messageBuffer The buffer that holds the message.
     */
    void processReceivedMessage(Client& client, Uint8 messageType,
                                std::span<Uint8> messageBuffer);

    /**
     * If the given client is awaiting authentication, sends the request's
     * ticket to the AccountServer for validation.
     */
    void processConnectionRequest(const std::shared_ptr<Client>& client,
                                  std::span<Uint8> messageBuffer);

    /**
     * Sends the given client a failed ConnectionResponse and marks it as
     * rejected.
     *
     * Note: We leave the connection open so the response can be sent. If the
     *       client doesn't disconnect, it'll be dropped when its auth timeout
     *       expires.
     */
    void rejectClient(Client& client, ConnectionResponse::Result result);

    /** Used to get the client map and current tick. */
    WorldClientEndpoint& endpoint;

    /** Used to push network events like connections/disconnections. */
    EventDispatcher& dispatcher;

    /** Used to process received messages. */
    MessageProcessor& messageProcessor;

    /** Used to validate the tickets that clients send us. */
    AccountServiceEndpoint& accountEndpoint;

    /** A ticket validation result, received from the AccountServer. */
    struct TicketResult {
        /** The client that sent the ticket. */
        NetworkID netID{0};
        /** Used to make sure the client didn't disconnect (and have its netID
            reused) while we were waiting. */
        std::weak_ptr<Client> client{};
        ConsumeServiceTicketResponse response{};
    };
    /** Holds ticket results until the receive thread can process them.
        Written to by the network IO thread, read by the receive thread. */
    moodycamel::ReaderWriterQueue<TicketResult> ticketResultQueue;

    /** Used for generating network IDs. */
    IDPool networkIDPool;

    /** The number of clients that are currently connected. */
    unsigned int clientCount;

    /** The socket set used for all clients. Lets us do select()-like behavior,
        allowing our receive thread to not be constantly spinning. */
    std::shared_ptr<SocketSet> clientSet;

    /** The listener that we use to accept new clients. */
    Acceptor acceptor;

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

} // End namespace WorldServer
} // End namespace AM
