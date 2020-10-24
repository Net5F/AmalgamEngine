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
#include "Log.h"

namespace AM
{
class Peer;
class ConnectionResponse;
class EntityUpdate;

namespace Client
{
/**
 * Provides Network functionality in the format that the Game wants.
 */
class Network
{
public:
    Network();

    ~Network();

    /**
     * Attempts to establish a connection with the server.
     * If successful, starts the receiver thread.
     *
     * @return true if a connection was successfully established, else false.
     */
    bool connect();

    /**
     * If no messages have been sent since the last heartbeat, sends a message.
     */
    void tick();

    /**
     * Sends bytes over the network.
     * Errors if the server is disconnected.
     */
    void send(const BinaryBufferSharedPtr& message);

    /**
     * Returns a message if there are any in the associated queue.
     * If there are none, waits for one up to the given timeout.
     *
     * @param timeoutMs  How long to wait. 0 for no wait, -1 for indefinite.
     *                   Defaults to 0.
     * @return A waiting message, else nullptr.
     */
    std::unique_ptr<ConnectionResponse>
        receiveConnectionResponse(Uint64 timeoutMs = 0);
    std::shared_ptr<const EntityUpdate> receivePlayerUpdate(Uint64 timeoutMs
                                                            = 0);
    NpcReceiveResult receiveNpcUpdate(Uint64 timeoutMs = 0);

    /**
     * Thread function, started from connect().
     * Tries to retrieve a message from the server.
     * If successful, passes it to queueMessage().
     */
    int pollForMessages();

    /**
     * Subtracts an appropriate amount from the tickAdjustment based on its
     * current value, and returns the amount subtracted.
     * @return 1 if there's a negative tickAdjustment (the sim can only freeze 1
     *         tick at a time), else 0 or a negative amount equal to the current
     *         tickAdjustment.
     */
    int transferTickAdjustment();

    /** Initialize the tick timer. */
    void initTimer();

    /** Used for passing us a pointer to the Game's currentTick. */
    void registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    void setNetstatsLoggingEnabled(bool inNetstatsLoggingEnabled);

private:
    /**
     * If we haven't sent any messages since the last network tick, sends a
     * heartbeat.
     */
    void sendHeartbeatIfNecessary();

    /**
     * Processes the received header and following batch.
     * If any messages are expected, receives the messages.
     * If it confirmed any ticks that had no changes, updates the confirmed tick
     * count.
     */
    void processBatch();

    /**
     * Pushes a message into the appropriate queue, based on its contents.
     * @pre A serialized message is in messageRecBuffer, starting at index 0.
     * @param messageType  The type of the received message to process.
     * @param messageSize  The length in bytes of the message in
     *                     messageRecBuffer.
     */
    void processReceivedMessage(MessageType messageType, Uint16 messageSize);

    /**
     * Checks if we need to process the received adjustment, does so if
     * necessary.
     * @param receivedTickAdj  The received tick adjustment.
     * @param receivedAdjIteration  The adjustment iteration for the received
     *                              adjustment.
     */
    void adjustIfNeeded(Sint8 receivedTickAdj, Uint8 receivedAdjIteration);

    /**
     * Logs the network stats such as bytes sent/received per second.
     */
    void logNetworkStatistics();

    /** Used to time when we should process the network tick. */
    Timer tickTimer;

    /** The aggregated time since we last processed a tick. */
    double accumulatedTime;

    std::shared_ptr<Peer> server;

    /** Local copy of the playerID so we can tell if we got a player message. */
    EntityID playerID;

    /** The adjustment that the server has told us to apply to the tick. */
    std::atomic<int> tickAdjustment;

    /**
     * Tracks what iteration of tick offset adjustments we're on.
     * Used to make sure that we don't double-count adjustments.
     */
    std::atomic<Uint8> adjustmentIteration;

    /**
     * Tracks if we've sent a message since the last network tick.
     * Used to determine if we need to heartbeat.
     */
    unsigned int messagesSentSinceTick;

    /** Pointer to the game's current tick. */
    const std::atomic<Uint32>* currentTickPtr;

    /** Tracks how long it's been since we've received a message from the
        server. */
    Timer receiveTimer;

    /** Calls pollForMessages(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;

    /** These queues store received messages that are waiting to be consumed. */
    using ConnectionResponseQueue = moodycamel::BlockingReaderWriterQueue<
        std::unique_ptr<ConnectionResponse>>;
    ConnectionResponseQueue connectionResponseQueue;

    using PlayerUpdateQueue = moodycamel::BlockingReaderWriterQueue<
        std::shared_ptr<const EntityUpdate>>;
    PlayerUpdateQueue playerUpdateQueue;

    using NpcUpdateQueue
        = moodycamel::BlockingReaderWriterQueue<NpcUpdateMessage>;
    NpcUpdateQueue npcUpdateQueue;

    /** Used to hold headers while we process them. */
    BinaryBuffer headerRecBuffer;
    /** Used to hold messages while we deserialize them. */
    BinaryBuffer messageRecBuffer;

    /** The number of seconds we'll wait before logging our network
        statistics. */
    static constexpr unsigned int SECONDS_TILL_STATS_DUMP = 5;
    static constexpr unsigned int TICKS_TILL_STATS_DUMP
        = (1 / NETWORK_TICK_TIMESTEP_S) * SECONDS_TILL_STATS_DUMP;

    /** Whether network statistics logging is enabled or not. */
    bool netstatsLoggingEnabled;

    /** The number of ticks since we last logged our network statistics. */
    unsigned int ticksSinceNetstatsLog;
};

} // namespace Client
} // namespace AM
