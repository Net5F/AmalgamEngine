#ifndef NETWORK_H
#define NETWORK_H

#include "SharedDefs.h"
#include <string>
#include <memory>
#include <cstddef>
#include <unordered_map>
#include <atomic>
#include <thread>
#include "readerwriterqueue.h"
#include "MessageSorter.h"

struct _SDLNet_SocketSet;
typedef struct _SDLNet_SocketSet* SDLNet_SocketSet;

namespace AM
{

class Acceptor;
class Peer;

namespace Server
{

class Network
{
public:
    static constexpr unsigned int MAX_CLIENTS = 100;
    static constexpr unsigned int MAX_QUEUED_NEW_CLIENTS = 100;

    /** 20 network ticks per second. */
    static constexpr float NETWORK_TICK_INTERVAL_S = 1 / 20.0f;

    Network();

    virtual ~Network();

    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     */
    void send(std::shared_ptr<Peer> client, BinaryBufferSharedPtr message);

    /**
     * Queues a message to be sent to all connected clients the next time
     * sendWaitingMessages is called.
     */
    void sendToAll(BinaryBufferSharedPtr message);

    /**
     * Sends any queued messages over the network.
     * Acts as the Network's tick.
     */
    void sendWaitingMessages(float deltaSeconds);

    /** Forwards to the inputMessageSorter's startReceive. */
    std::queue<BinaryBufferSharedPtr>& startReceiveInputMessages(Uint32 tickNum);

    /** Forward to the inputMessageSorter's endReceive. */
    void endReceiveInputMessages();

    /**
     * Accepts any new clients, pushing them into the newClientQueue.
     * Note: DOES NOT add new clients to the client map. You must tie them to an
     *       entityID through addClient().
     */
    void acceptNewClients();

    /**
     * Adds the given client to the client map at key entityID.
     */
    void addClient(EntityID entityID, std::shared_ptr<Peer> client);

    /**
     * @return A pointer to a new client if one is waiting, else nullptr.
     */
    std::shared_ptr<Peer> getNewClient();

    const std::shared_ptr<SDLNet_SocketSet> getClientSet();

    const std::unordered_map<EntityID, std::shared_ptr<Peer>>& getClients();

    std::atomic<bool> const* getExitRequestedPtr();

private:
    /** Pairs a message with the client to send it to. client == nullptr means send to all. */
    struct MessageInfo {
        std::shared_ptr<Peer> client;
        BinaryBufferSharedPtr message;
    };

    /**
     * Iterates through the clients and erases any that are disconnected.
     */
    void eraseDisconnectedClients();

    /**
     * Thread function, started from constructor.
     * Accepts new clients, erases disconnected clients, and tries to receive
     * messages from the clients.
     */
    static int processClients(Network* network);

    /**
     * Used by pollForMessages, checks for new messages and pushes them into their queues.
     */
    static void receiveClientMessages(
    Network* network, const std::shared_ptr<SDLNet_SocketSet> clientSet,
    const std::unordered_map<EntityID, std::shared_ptr<Peer>>& clients);

    /**
     * Pushes a message into the inputQueue.
     */
    void queueInputMessage(BinaryBufferSharedPtr message);

    /**
     * Tries to send any messages in sendQueue over the network.
     * If a send fails, leaves the message at the front of the queue and returns.
     */
    void sendWaitingMessagesInternal();


    static const std::string SERVER_IP;
    static constexpr int SERVER_PORT = 41499;

    std::unique_ptr<Acceptor> acceptor;
    /** Stores new connected clients until the game ties them to an ID and
        adds them to the clients map. */
    moodycamel::ReaderWriterQueue<std::shared_ptr<Peer>> newClientQueue;

    /** The socket set used for all clients. Lets us do select()-like behavior,
        allowing our receive thread to not be constantly spinning. */
    std::shared_ptr<SDLNet_SocketSet> clientSet;
    /** Maps IDs to their connections. Allows the game to say "send this message
        to this entity" instead of needing to track the connection objects. */
    std::unordered_map<EntityID, std::shared_ptr<Peer>> clients;

    /** The outgoing message queue. Holds messages ready to be sent. */
    std::deque<MessageInfo> sendQueue;

    /** The aggregated time since we last processed a tick. */
    float accumulatedTime;

    /** Stores input messages from clients, sorted by tick number. */
    MessageSorter inputMessageSorter;

    /** Calls processClients(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;
};

} // namespace Server
} // namespace AM

#endif /* NETWORK_H */
