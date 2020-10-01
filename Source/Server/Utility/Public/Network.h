#pragma once

#include "NetworkDefs.h"
#include "ServerNetworkDefs.h"
#include "ClientHandler.h"
#include "MessageSorter.h"
#include <memory>
#include <cstddef>
#include <unordered_map>
#include <shared_mutex>
#include <queue>
#include "readerwriterqueue.h"

namespace AM
{

class Acceptor;
class Peer;
class ClientInputs;

namespace Server
{

/**
 * Provides Network functionality in the format that the Game wants.
 */
class Network
{
public:
    static constexpr unsigned int SERVER_PORT = 41499;

    Network();

    /**
     * Sends any queued messages over the network.
     */
    void tick();

    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     * @throws std::out_of_range if id is not in the clients map.
     *
     * @param networkID  The client to send the message to.
     * @param message  The message to send.
     * @param messageTick  Optional, for messages that are associated with a tick number.
     *                     Used to update the client's latestSentSimTick.
     */
    void send(NetworkID networkID, const BinaryBufferSharedPtr& message,
              Uint32 messageTick = 0);

    /**
     * Deserializes and routes received client messages.
     *
     * When a message with a tick number is received, updates the associated client's
     * tick diff data.
     *
     * @param receiveQueue  A queue with messages to process.
     */
    void processReceivedMessages(std::queue<ClientMessage>& receiveQueue);

    /** Forwards to the inputMessageSorter's startReceive. */
    std::queue<std::unique_ptr<ClientInputs>>& startReceiveInputMessages(Uint32 tickNum);

    /** Forward to the inputMessageSorter's endReceive. */
    void endReceiveInputMessages();

    /** Initialize the tick timer. */
    void initTimer();

    // Returning non-const refs because they need to be modified. Be careful not to attempt
    // to re-assign the obtained ref (can't re-seat a reference once bound).
    ClientMap& getClientMap();
    std::shared_mutex& getClientMapMutex();

    moodycamel::ReaderWriterQueue<NetworkID>& getConnectEventQueue();
    moodycamel::ReaderWriterQueue<NetworkID>& getDisconnectEventQueue();

    /** Used for passing us a pointer to the Game's currentTick. */
    void registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * Allocates and fills a dynamic buffer with a message header and data payload.
     *
     * The first byte of the buffer will contain the message type as a Uint8.
     * The next 2 bytes will contain the message size as a Uint16.
     * The rest will have the data from the given messageBuffer copied into it.
     */
    static BinaryBufferSharedPtr constructMessage(MessageType type, Uint8* messageBuffer,
                                                  std::size_t size);

private:
    /**
     * Tries to send any messages in each client's queue over the network.
     * If a send fails, leaves the message at the front of the queue and moves on to the
     * next client's queue.
     * If there's no messages to send, sends a heartbeat instead, with a value that confirms
     * that we've processed tick(s) with no changes to send.
     */
    void sendClientUpdates();

    /** Used to time when we should process the network tick. */
    Timer tickTimer;

    /** The aggregated time since we last processed a tick. */
    double accumulatedTime;

    /** Maps IDs to their connections. Allows the game to say "send this message
        to this entity" instead of needing to track the connection objects. */
    ClientMap clientMap;
    std::shared_mutex clientMapMutex;

    ClientHandler clientHandler;

    /** These queues are used to inform the game of connection events. */
    moodycamel::ReaderWriterQueue<NetworkID> connectEventQueue;
    moodycamel::ReaderWriterQueue<NetworkID> disconnectEventQueue;

    /** Stores input messages received from clients, sorted by tick number. */
    MessageSorter<std::unique_ptr<ClientInputs>> inputMessageSorter;

    /** Pointer to the game's current tick. */
    const std::atomic<Uint32>* currentTickPtr;
};

} // namespace Server
} // namespace AM
