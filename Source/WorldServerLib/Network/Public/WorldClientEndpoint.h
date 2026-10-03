#pragma once

#include "SharedConfig.h"
#include "NetworkID.h"
#include "ClientMap.h"
#include "MessageProcessor.h"
#include "ClientHandler.h"
#include "Serialize.h"
#include "Peer.h"
#include "ByteTools.h"
#include "QueuedEvents.h"
#include "tracy/Tracy.hpp"
#include <memory>
#include <cstddef>
#include <unordered_map>
#include <shared_mutex>

namespace AM
{
class Acceptor;

namespace WorldServer
{
struct MessageProcessorContext;
class IMessageProcessorExtension;

/**
 * Manages communication with clients (ran by users).
 *
 * Provides a convenient interface for sending and receiving messages, and
 * other network-related functionality.
 *
 * Internally, manages client connections and orchestrates message sending
 * and receiving.
 *
 * TODO: Add comment about how this connection behaves (see
 *       AccountClientEndpoint.h).
 */
class WorldClientEndpoint
{
public:
    WorldClientEndpoint(
        const MessageProcessorContext& inMessageProcessorContext);

    /**
     * Sends all queued messages over the network.
     *
     * Also logs network statistics periodically.
     */
    void tick();

    /**
     * Sends a message over the network.
     * Equivalent to calling serialize() and sendBytes().
     *
     * @param networkID The client to send the message to.
     * @param message A structure that defines MESSAGE_TYPE and has an
     * associated serialize() function.
     * @param messageTick Optional, used in certain cases to update the
     * Client's latestSentSimTick.
     */
    template<typename Message>
    void send(NetworkID networkID, const Message& message,
              Uint32 messageTick = 0);

    /**
     * Serializes and frames the given message.
     *
     * @param message A structure that defines MESSAGE_TYPE and has an
     *                associated serialize() function.
     * @return A message that's ready to be passed to sendBytes().
     */
    template<typename Message>
    BinaryBufferSharedPtr serialize(const Message& message);

    /**
     * Queues a serialized message to be sent the next time
     * sendWaitingMessages is called.
     * @throws std::out_of_range if id is not in the clients map.
     *
     * @param networkID The client to send the message to.
     * @param message The message to send.
     * @param messageTick Optional, used when sending entity movement updates
     * to update the Client's latestSentSimTick.
     */
    void sendBytes(NetworkID networkID, const BinaryBufferSharedPtr& message,
                   Uint32 messageTick = 0);

    /**
     * Returns the endpoint's event dispatcher. All messages that we receive
     * from clients are pushed into this dispatcher.
     */
    EventDispatcher& getEventDispatcher();

    // Returning non-const refs because they need to be modified. Be careful not
    // to attempt to re-assign the obtained ref (can't re-seat a reference once
    // bound).
    ClientMap& getClientMap();
    SharedLockableBase(std::shared_mutex) & getClientMapMutex();

    /** Used for passing us a pointer to the sim's currentTick. */
    void registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /** Convenience for network-owned objects to get the current tick. */
    Uint32 getCurrentTick();

    /**
     * See MessageProcessor::extension member comment.
     */
    void setMessageProcessorExtension(IMessageProcessorExtension* extension);

private:
    /**
     * Logs the network stats such as bytes sent/received per second.
     */
    void logNetworkStatistics();

    /** Maps IDs to their connections. Allows the game to say "send this message
        to this entity" instead of needing to track the connection objects. */
    ClientMap clientMap;

    /** Used to lock access to the clientMap. */
    TracySharedLockable(std::shared_mutex, clientMapMutex);

    /** Used to dispatch events from the network to the simulation. */
    EventDispatcher eventDispatcher;

    /** Deserializes messages, does any network-layer message handling, and
        passes messages down to the simulation. */
    MessageProcessor messageProcessor;

    /** Handles asynchronous client activity. */
    ClientHandler clientHandler;

    /** The number of seconds we'll wait before logging our network
        statistics. */
    static constexpr unsigned int SECONDS_TILL_STATS_DUMP{5};
    static constexpr unsigned int TICKS_TILL_STATS_DUMP{
        static_cast<unsigned int>(
            (1 / SharedConfig::SERVER_NETWORK_TICK_TIMESTEP_S)
            * SECONDS_TILL_STATS_DUMP)};

    /** The number of ticks since we last logged our network statistics. */
    unsigned int ticksSinceNetstatsLog;

    /** Pointer to the sim's current tick. */
    const std::atomic<Uint32>* currentTickPtr;
};

template<typename Message>
void WorldClientEndpoint::send(NetworkID networkID, const Message& message,
                               Uint32 messageTick)
{
    // Serialize and frame the message.
    BinaryBufferSharedPtr messageBuffer{serialize(message)};

    // Send the message.
    sendBytes(networkID, messageBuffer, messageTick);
}

template<typename Message>
BinaryBufferSharedPtr WorldClientEndpoint::serialize(const Message& message)
{
    // Allocate the buffer.
    std::size_t totalMessageSize{MESSAGE_HEADER_SIZE
                                 + Serialize::measureSize(message)};
    BinaryBufferSharedPtr messageBuffer{
        std::make_shared<BinaryBuffer>(totalMessageSize)};

    // Serialize the message struct into the buffer, leaving room for the
    // header.
    std::size_t messageSize{Serialize::toBuffer(messageBuffer->data(),
                                                messageBuffer->size(), message,
                                                MESSAGE_HEADER_SIZE)};

    // Copy the type into the buffer.
    // TODO: Add a nice compile-time message if Message doesn't define
    //       MESSAGE_TYPE.
    messageBuffer->at(MessageHeaderIndex::MessageType)
        = static_cast<Uint8>(Message::MESSAGE_TYPE);

    // Copy the messageSize into the buffer.
    ByteTools::write16(static_cast<Uint16>(messageSize),
                       (messageBuffer->data() + MessageHeaderIndex::Size));

    return messageBuffer;
}

} // namespace WorldServer
} // namespace AM
