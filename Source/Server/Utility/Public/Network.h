#ifndef NETWORK_H
#define NETWORK_H

#include "Client.h"
#include "NetworkDefs.h"
#include "MessageSorter.h"
#include "Message_generated.h"
#include <string>
#include <memory>
#include <cstddef>
#include <unordered_map>
#include <atomic>
#include <thread>
#include "readerwriterqueue.h"

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

    static void registerCurrentTickPtr(Uint32* inCurrentTickPtr);

    Network();

    virtual ~Network();

    // TODO: Can we merge some of the socket stuff into Client?
    //       Or some of the "connected but not in the game state yet" stuff?
    //       Need to clarify difference between Peer and Client and
    //       rename the functions that call a shared_ptr<Peer> a client.

    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     */
    void send(EntityID id, BinaryBufferSharedPtr message);

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
     * Adds the given client to the client map at key entityID
     *
     * Effectively, this means it will start receiving messages broadcast
     * through sendToAll().
     *
     * Do this after the client has been registered with the game.
     */
    void addClient(EntityID entityID, std::shared_ptr<Peer> client);

    /**
     * Queues the given data to be sent on the next network tick to the client associated
     * with the given EntityID.
     *
     * This must be done instead of queueing a message immediately to ensure that the
     * client receives the latest tick (because messages are batched).
     */
    void sendConnectionResponse(EntityID id, float spawnX, float spawnY);

    /**
     * @return A pointer to a new client if one is waiting, else nullptr.
     */
    std::shared_ptr<Peer> getNewClient();

    const std::shared_ptr<SDLNet_SocketSet> getClientSet();

    std::unordered_map<EntityID, Client>& getClients();

    std::atomic<bool> const* getExitRequestedPtr();

private:
    /** Holds data for a deferred send of a ConnectionResponse message. */
    struct ConnectionResponseData {
        EntityID id;
        float spawnX;
        float spawnY;
    };

    /**
     * Thread function, started from constructor.
     * Accepts new clients and tries to receive messages from the clients.
     */
    static int processClients(Network* network);

    /**
     * Used by pollForMessages, checks for new messages and pushes them into their queues.
     */
    static void receiveClientMessages(
    Network* network, const std::shared_ptr<SDLNet_SocketSet> clientSet,
    std::unordered_map<EntityID, Client>& clients);

    /**
     * Pushes a message into the inputQueue.
     */
    void queueInputMessage(BinaryBufferSharedPtr message);

    /**
     * Constructs any data in connectionResponseQueue into a message and
     * queues it in the relevant client's send queue.
     */
    void queueConnectionResponses();

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
    std::unordered_map<EntityID, Client> clients;

    /** The aggregated time since we last processed a tick. */
    float accumulatedTime;

    /** Stores input messages received from clients, sorted by tick number. */
    MessageSorter inputMessageSorter;

    /** Holds data for ConnectionResponse messages that need to be sent. */
    std::queue<ConnectionResponseData> connectionResponseQueue;

    /** Calls processClients(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;

    static constexpr int BUILDER_BUFFER_SIZE = 512;
    flatbuffers::FlatBufferBuilder builder;

    static Uint32* currentTickPtr;
};

} // namespace Server
} // namespace AM

#endif /* NETWORK_H */
