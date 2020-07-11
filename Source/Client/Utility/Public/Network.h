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
    /** If true, the connection to the server will be mocked and we'll run without it. */
    static constexpr bool RUN_OFFLINE = true;

    /** Our best guess at a good starting place for the simulation's tick offset. */
    static constexpr Sint8 INITIAL_TICK_OFFSET = 5;

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

    /**
     * Subtracts an appropriate amount from the tickAdjustment based on its current value,
     * and returns the amount subtracted.
     * @return 1 if there's a negative tickAdjustment (the sim can only freeze 1 tick at a
     *         time), else 0 or a negative amount equal to the current tickAdjustment.
     */
    int transferTickAdjustment();

    std::atomic<bool> const* getExitRequestedPtr();

private:
    static const std::string SERVER_IP;
    static constexpr unsigned int SERVER_PORT = 41499;

    /** How long we should wait before considering the server to be timed out. */
    static constexpr float TIMEOUT_S = NETWORK_TICK_INTERVAL_S * 2;

    std::shared_ptr<Peer> server;

    /** Local copy of the playerID so we can tell if we got a player message. */
    EntityID playerID;

    /**
     * The adjustment that the server has told us to apply to the tick.
     */
    std::atomic<int> tickAdjustment;

    /**
     * Tracks what iteration of tick offset adjustments we're on.
     * Used to make sure that we don't double-count adjustments.
     */
    std::atomic<Uint8> adjustmentIteration;

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
