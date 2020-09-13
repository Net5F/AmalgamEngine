#ifndef NETWORK_H
#define NETWORK_H

#include "NetworkDefs.h"
#include "ClientHandler.h"
#include "MessageSorter.h"
#include "Message_generated.h"
#include <string>
#include <memory>
#include <cstddef>
#include <unordered_map>
#include <shared_mutex>
#include "readerwriterqueue.h"

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
     */
    void send(NetworkID networkID, const BinaryBufferSharedPtr& message);

    /**
     * Queues a message to be sent to all connected clients the next time
     * sendWaitingMessages is called.
     *
     * @param message  The message to send.
     */
    void sendToAll(const BinaryBufferSharedPtr& message);

    /**
     * Pushes a message into the inputMessageSorter.
     * For use in receiving input messages.
     *
     * @param message  The message to send.
     * @return The amount that message's tickNum was ahead or behind the current tick.
     */
    Sint64 queueInputMessage(BinaryBufferPtr message);

    /** Forwards to the inputMessageSorter's startReceive. */
    std::queue<BinaryBufferPtr>& startReceiveInputMessages(Uint32 tickNum);

    /** Forward to the inputMessageSorter's endReceive. */
    void endReceiveInputMessages();

    /** Initialize the send timer. */
    void initTimer();

    // Returning non-const refs because they need to be modified. Be careful not to attempt
    // to re-assign the obtained ref (can't re-seat a reference once bound).
    std::unordered_map<NetworkID, Client>& getClientMap();
    std::shared_mutex& getClientMapMutex();
    moodycamel::ReaderWriterQueue<NetworkID>& getConnectEventQueue();
    moodycamel::ReaderWriterQueue<NetworkID>& getDisconnectEventQueue();

    /** Used for passing us a pointer to the Game's currentTick. */
    void registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * Allocates and fills a dynamic buffer with message data.
     *
     * The first 2 bytes of the buffer will contain the message size as a Uint16,
     * the rest will have the data at messageBuffer copied into it.
     *
     * For use with the Message type in our flatbuffer scheme. We aim to send the whole
     * Message + size with one send call, so it's convenient to have it all in
     * one buffer.
     */
    static BinaryBufferSharedPtr constructMessage(Uint8* messageBuffer, std::size_t size);

private:
    /**
     * Tries to send any messages in each client's queue over the network.
     * If a send fails, leaves the message at the front of the queue and moves on to the
     * next client's queue.
     * If there's no messages to send, sends a heartbeat instead, with a value that confirms
     * that we've processed tick(s) with no changes to send.
     */
    void sendClientUpdates();

    /** Used to time when we should send waiting messages. */
    Timer sendTimer;

    /** The aggregated time since we last processed a tick. */
    double accumulatedTime;

    /** Maps IDs to their connections. Allows the game to say "send this message
        to this entity" instead of needing to track the connection objects. */
    std::unordered_map<NetworkID, Client> clientMap;

    std::shared_mutex clientMapMutex;

    ClientHandler clientHandler;

    /** These queues are used to inform the game of connection events. */
    moodycamel::ReaderWriterQueue<NetworkID> connectEventQueue;
    moodycamel::ReaderWriterQueue<NetworkID> disconnectEventQueue;

    /** Stores input messages received from clients, sorted by tick number. */
    MessageSorter inputMessageSorter;

    static constexpr int BUILDER_BUFFER_SIZE = 512;
    flatbuffers::FlatBufferBuilder builder;

    /** Pointer to the game's current tick. */
    const std::atomic<Uint32>* currentTickPtr;
};

} // namespace Server
} // namespace AM

#endif /* NETWORK_H */
