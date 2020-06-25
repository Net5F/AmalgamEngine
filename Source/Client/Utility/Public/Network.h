#ifndef NETWORK_H
#define NETWORK_H

#include "GameDefs.h"
#include "NetworkDefs.h"
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <queue>
#include "readerwriterqueue.h"
#include "Timer.h"

namespace AM
{

class Peer;

namespace Client
{

/**
 * The various types of flatbuffer message that we receive.
 * Enum class because it doesn't read well from other namespaces.
 */
enum class MessageType {
    ConnectionResponse,
    PlayerUpdate,
    NpcUpdate
};

class Network
{
public:
    Network();

    virtual ~Network();

    bool connect();

    /**
     * Registers an entity as being the player. Various systems will only apply to this entity.
     */
    void registerPlayerID(EntityID inPlayerID);

    /**
     * Sends bytes over the network.
     * Errors if the server is disconnected.
     */
    void send(const BinaryBufferSharedPtr& message);

    /**
     * Returns a message if there are any in the requested queue.
     * If there are none, waits for one up to the given timeout.
     *
     * @param type  The type of message to receive.
     * @param timeoutMs  How long to wait. 0 for no wait, -1 for indefinite. Defaults to 0.
     * @return A waiting message, else nullptr.
     */
    BinaryBufferSharedPtr receive(MessageType type, Uint64 timeoutMs = 0);

    /**
     * Thread function, started from connect().
     * Tries to retrieve a message from the server.
     * If successful, passes it to queueMessage().
     */
    int pollForMessages();

    /**
     * Pushes a message into the appropriate queue, based on its contents.
     */
    void processReceivedMessage(BinaryBufferPtr messageBuffer);

    std::shared_ptr<Peer> getServer();

    Uint8 getTickOffset(bool fromSameThread = false);

    /** Records the tick offset associated with the iteration currentTick. */
    void recordTickOffset(Uint32 currentTick, Uint8 tickOffset);
    /** Returns the offset associated with the iteration tickNum. */
    Uint8 retrieveOffsetAtTick(Uint32 tickNum);

    std::atomic<bool> const* getExitRequestedPtr();

private:
    static const std::string SERVER_IP;
    static constexpr int SERVER_PORT = 41499;

    /** Our best guess at a good starting place for the tick offset. */
    static constexpr Sint8 STARTING_TICK_OFFSET = 5;

    /** How long we should wait before considering the server to be timed out. */
    static constexpr float TIMEOUT_S = NETWORK_TICK_INTERVAL_S * 2;

    std::shared_ptr<Peer> server;

    /** Local copy of the playerID so we can tell if we got a player message. */
    EntityID playerID;

    /**
     * The offset that we apply to the Game's currentTick when we build a message.
     * Effectively puts us "in the future" from the Server's view, so that it can buffer
     * our messages and have them ready when it's processing each tick.
     */
    std::atomic<Uint8> tickOffset;

    /**
     * The iteration number of the latest tick offset adjustment we've received.
     * Used to make sure that we don't double-count adjustments.
     */
    std::atomic<Uint8> adjustmentIteration;

    struct TickOffsetMapping {
        /** The tick that this client was at during capture. */
        Uint32 tickNum;
        /** The forward-dated tick that was sent to the server. */
        Uint8 tickOffset;
    };
    /**
     * Tracks the tickOffset used in each message, so that it can later be reversed
     * for client-side prediction.
     */
    std::queue<TickOffsetMapping> tickOffsetHistory;

    /** Tracks how long it's been since we've received a message from the server. */
    Timer receiveTimer;

    /** Calls pollForMessages(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;

    /** These queues store received messages that are waiting to be consumed. */
    typedef moodycamel::BlockingReaderWriterQueue<BinaryBufferSharedPtr> MessageQueue;
    MessageQueue connectionResponseQueue;
    MessageQueue playerUpdateQueue;
    MessageQueue npcUpdateQueue;
};

} // namespace Client
} // namespace AM

#endif /* NETWORK_H */
