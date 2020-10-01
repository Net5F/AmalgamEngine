#pragma once

#include "GameDefs.h"
#include "NetworkDefs.h"
#include "ClientNetworkDefs.h"
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include "readerwriterqueue.h"
#include "Timer.h"
#include "Debug.h"

namespace AM
{

class Peer;
class ConnectionResponse;
class EntityUpdate;

namespace Client
{

class Network
{
public:
    Network();

    ~Network();

    bool connect();

    /**
     * Sends bytes over the network.
     * Errors if the server is disconnected.
     */
    void send(const BinaryBufferSharedPtr& message);

    /**
     * Returns a message if there are any in the associated queue.
     * If there are none, waits for one up to the given timeout.
     *
     * @param timeoutMs  How long to wait. 0 for no wait, -1 for indefinite. Defaults to 0.
     * @return A waiting message, else nullptr.
     */
    std::unique_ptr<ConnectionResponse> receiveConnectionResponse(Uint64 timeoutMs = 0);
    std::shared_ptr<const EntityUpdate> receivePlayerUpdate(Uint64 timeoutMs = 0);
    NpcReceiveResult receiveNpcUpdate(Uint64 timeoutMs = 0);

    /**
     * Thread function, started from connect().
     * Tries to retrieve a message from the server.
     * If successful, passes it to queueMessage().
     */
    int pollForMessages();

    /**
     * Subtracts an appropriate amount from the tickAdjustment based on its current value,
     * and returns the amount subtracted.
     * @return 1 if there's a negative tickAdjustment (the sim can only freeze 1 tick at a
     *         time), else 0 or a negative amount equal to the current tickAdjustment.
     */
    int transferTickAdjustment();

    /**
     * Fills the message header (message type and size) into the given buffer.
     * Also shrinks the buffer to fit the content, if it's over-sized.
     *
     * The first CLIENT_HEADER_SIZE bytes of the buffer will be left empty to later be
     * filled with the client header by the network before sending.
     *
     * The first byte following the client header will contain the message type as a Uint8.
     * The next 2 bytes will contain the message size as a Uint16.
     * The rest will have the data from the given messageBuffer copied into it.
     */
    static void fillMessageHeader(MessageType type, std::size_t messageSize,
                                  const BinaryBufferSharedPtr& messageBuffer);

private:
    /**
     * Processes the received header and following batch.
     * If any messages are expected, receives the messages.
     * If it confirmed any ticks that had no changes, updates the confirmed tick count.
     */
    void processBatch();

    /**
     * Pushes a message into the appropriate queue, based on its contents.
     * @param messageType  The type of the received message to process.
     * @param messageSize  The size to use in deserializing the message in messageRecBuffer.
     */
    void processReceivedMessage(MessageType messageType, Uint16 messageSize);

    /**
     * Checks if we need to process the received adjustment, does so if necessary.
     * @param receivedTickAdj  The received tick adjustment.
     * @param receivedAdjIteration  The adjustment iteration for the received adjustment.
     */
    void adjustIfNeeded(Sint8 receivedTickAdj, Uint8 receivedAdjIteration);

    std::shared_ptr<Peer> getServer() const;
    std::atomic<bool> const* getExitRequestedPtr() const;

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
    using ConnectionResponseQueue
              = moodycamel::BlockingReaderWriterQueue<std::unique_ptr<ConnectionResponse>>;
    ConnectionResponseQueue connectionResponseQueue;

    using PlayerUpdateQueue
              = moodycamel::BlockingReaderWriterQueue<std::shared_ptr<const EntityUpdate>>;
    PlayerUpdateQueue playerUpdateQueue;

    using NpcUpdateQueue
              = moodycamel::BlockingReaderWriterQueue<NpcUpdateMessage>;
    NpcUpdateQueue npcUpdateQueue;

    /** Used to hold headers while we process them. */
    BinaryBuffer headerRecBuffer;
    /** Used to hold messages while we deserialize them. */
    BinaryBuffer messageRecBuffer;
};

} // namespace Client
} // namespace AM
