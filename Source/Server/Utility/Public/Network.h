#ifndef NETWORK_H
#define NETWORK_H

#include "NetworkDefs.h"
#include "Client.h"
#include "MessageSorter.h"
#include "Message_generated.h"
#include <string>
#include <memory>
#include <cstddef>
#include <atomic>
#include <unordered_map>
#include <shared_mutex>
#include "readerwriterqueue.h"

struct _SDLNet_SocketSet;
typedef struct _SDLNet_SocketSet* SDLNet_SocketSet;

namespace AM
{

class Acceptor;
class Peer;

namespace Server
{

/**
 * Provides Network functionality in the format that the Game wants.
 */
class Network
{
public:
    /** 20 network ticks per second. */
    static constexpr float NETWORK_TICK_INTERVAL_S = 1 / 20.0f;

    static constexpr int SERVER_PORT = 41499;

    static void registerCurrentTickPtr(Uint32* inCurrentTickPtr);

    Network();

    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     * @throws std::out_of_range if id is not in the clients map.
     */
    void send(NetworkID id, BinaryBufferSharedPtr message);

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

    /**
     * Pushes a message into the inputMessageSorter.
     * For use in receiving input messages.
     */
    void queueInputMessage(BinaryBufferSharedPtr message);

    /** Forwards to the inputMessageSorter's startReceive. */
    std::queue<BinaryBufferSharedPtr>& startReceiveInputMessages(Uint32 tickNum);

    /** Forward to the inputMessageSorter's endReceive. */
    void endReceiveInputMessages();

    /**
     * Queues the given data to be sent on the next network tick to the client associated
     * with the given NetworkID.
     *
     * This must be done instead of queueing a message immediately to ensure that the
     * client receives the latest tick (because messages are batched).
     */
    void sendConnectionResponse(NetworkID id, float spawnX, float spawnY);

    std::unordered_map<NetworkID, Client>& getClientMap();
    std::shared_mutex& getClientMapMutex();
    moodycamel::ReaderWriterQueue<NetworkID>& getConnectEventQueue();

private:
    /** Holds data for a deferred send of a ConnectionResponse message. */
    struct ConnectionResponseData {
        NetworkID id;
        float spawnX;
        float spawnY;
    };

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

    /** The aggregated time since we last processed a tick. */
    float accumulatedTime;

    /** Maps IDs to their connections. Allows the game to say "send this message
        to this entity" instead of needing to track the connection objects. */
    std::unordered_map<NetworkID, Client> clientMap;

    std::shared_mutex clientMapMutex;

    moodycamel::ReaderWriterQueue<NetworkID> connectEventQueue;

    /** Stores input messages received from clients, sorted by tick number. */
    MessageSorter inputMessageSorter;

    /** Holds data for ConnectionResponse messages that need to be sent. */
    std::queue<ConnectionResponseData> connectionResponseQueue;

    static constexpr int BUILDER_BUFFER_SIZE = 512;
    flatbuffers::FlatBufferBuilder builder;

    /** Pointer to the game's current tick. */
    static Uint32* currentTickPtr;
};

} // namespace Server
} // namespace AM

#endif /* NETWORK_H */
