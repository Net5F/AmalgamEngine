#pragma once

#include "SharedConfig.h"
#include "NetworkDefs.h"
#include "ServerNetworkDefs.h"
#include "MessageProcessor.h"
#include "ClientHandler.h"
#include "Serialize.h"
#include "Peer.h"
#include "ByteTools.h"
#include "QueuedEvents.h"
#include <memory>
#include <cstddef>
#include <unordered_map>
#include <shared_mutex>

namespace AM
{
class Acceptor;

namespace Server
{
/**
 * Provides network functionality in the format that the Simulation wants.
 */
class Network
{
public:
    static constexpr unsigned int SERVER_PORT = 41499;

    Network(EventDispatcher& inNetworkEventDispatcher);

    /**
     * Sends all queued messages over the network.
     *
     * Also logs network statistics if it's time to do so.
     */
    void tick();

    /**
     * Sends bytes over the network.
     * Errors if the server is disconnected.
     *
     * @param networkID  The client to send the message to.
     * @param messageStruct  A structure that defines MESSAGE_TYPE and has an
     *                       associated serialize() function.
     * @param messageTick  Optional, used in certain cases to update the
     *                     Client's latestSentSimTick.
     */
    template<typename T>
    void serializeAndSend(NetworkID networkID, const T& messageStruct,
                          Uint32 messageTick = 0);

    /** Initialize the tick timer. */
    void initTimer();

    // Returning non-const refs because they need to be modified. Be careful not
    // to attempt to re-assign the obtained ref (can't re-seat a reference once
    // bound).
    ClientMap& getClientMap();
    std::shared_mutex& getClientMapMutex();

    /** Used for passing us a pointer to the Game's currentTick. */
    void registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * Returns how much time in seconds is left until the next heartbeat.
     */
    double getTimeTillNextHeartbeat();

    /** Convenience for network-owned objects to get the current tick. */
    Uint32 getCurrentTick();

private:
    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     * @throws std::out_of_range if id is not in the clients map.
     *
     * @param networkID  The client to send the message to.
     * @param message  The message to send.
     * @param messageTick  Optional, used when sending entity movement updates
     *                     to update the Client's latestSentSimTick.
     */
    void send(NetworkID networkID, const BinaryBufferSharedPtr& message,
              Uint32 messageTick = 0);

    /**
     * Logs the network stats such as bytes sent/received per second.
     */
    void logNetworkStatistics();

    /** Maps IDs to their connections. Allows the game to say "send this message
        to this entity" instead of needing to track the connection objects. */
    ClientMap clientMap;

    /** Used to lock access to the clientMap. */
    std::shared_mutex clientMapMutex;

    /** Deserializes messages, does any network-layer message handling, and
        passes messages down to the simulation. */
    MessageProcessor messageProcessor;

    /** Handles asynchronous client activity. */
    ClientHandler clientHandler;

    /** The number of seconds we'll wait before logging our network
        statistics. */
    static constexpr unsigned int SECONDS_TILL_STATS_DUMP = 5;
    static constexpr unsigned int TICKS_TILL_STATS_DUMP
        = (1 / SharedConfig::NETWORK_TICK_TIMESTEP_S) * SECONDS_TILL_STATS_DUMP;

    /** The number of ticks since we last logged our network statistics. */
    unsigned int ticksSinceNetstatsLog;

    /** Pointer to the game's current tick. */
    const std::atomic<Uint32>* currentTickPtr;
};

template<typename T>
void Network::serializeAndSend(NetworkID networkID, const T& messageStruct,
                               Uint32 messageTick)
{
    // Allocate the buffer.
    BinaryBufferSharedPtr messageBuffer
        = std::make_shared<BinaryBuffer>(Peer::MAX_WIRE_SIZE);

    // Serialize the message struct into the buffer, leaving room for the
    // header.
    std::size_t messageSize = Serialize::toBuffer(messageBuffer->data(), messageBuffer->size(), messageStruct,
                                                  MESSAGE_HEADER_SIZE);

    // Copy the type into the buffer.
    // TODO: Add a nice compile-time message if T doesn't have MESSAGE_TYPE.
    messageBuffer->at(MessageHeaderIndex::MessageType)
        = static_cast<Uint8>(T::MESSAGE_TYPE);

    // Copy the messageSize into the buffer.
    const unsigned int totalMessageSize = MESSAGE_HEADER_SIZE + messageSize;
    ByteTools::write16(messageSize,
                       (messageBuffer->data() + MessageHeaderIndex::Size));

    // Shrink the buffer to fit.
    messageBuffer->resize(totalMessageSize);

    // Send the message.
    send(networkID, messageBuffer, messageTick);
}

} // namespace Server
} // namespace AM
